#include <fcntl.h>
#include <pty.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <filesystem>

// Структура для хранения информации о файлах
struct FileInfo {
    int permissions;
    std::string owner;
    std::string group;
    long size;
    std::filesystem::file_time_type modified_time;
    std::filesystem::path path;
    bool is_directory;
};

class SftpClient {
    int master_fd;
    pid_t pid;

public:
    SftpClient() : master_fd(-1), pid(-1) {}

    bool openApp(const char* app, const char* arg) {
        pid = forkpty(&master_fd, nullptr, nullptr, nullptr);

        if (pid < 0) {
            std::cerr << "forkpty failed\n";
            return false;
        } else if (pid == 0) {
            // Child process
            execlp(app, app, arg, (char*) nullptr);
            std::cerr << "execlp failed\n";
            return false;
        }

        return true;
    }

    std::vector<std::string> waitFor(const char* successPattern, const std::vector<const char*>& errorPatterns) {
        char buf[1024];
        ssize_t n;
        std::vector<std::string> lines;

        while ((n = read(master_fd, buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';
            std::cout << "Received: " << buf << std::endl;
            lines.push_back(buf);

            for (const auto& pattern : errorPatterns) {
                fprintf(stderr, "checking error pattern %s\n", pattern);
                if (std::strstr(buf, pattern)) {
                    std::cerr << "Error pattern detected: " << pattern << std::endl;
                    return {};
                }
            }

            if (std::strstr(buf, successPattern)) {
                fprintf(stderr, "found successPattern\n");
                break;
            }
        }

        if (n < 0) {
            std::cerr << "read failed\n";
        }

        std::vector<std::string> result;
        for (const auto& str : lines) {
            std::stringstream ss(str);
            std::string token;
            while (std::getline(ss, token, '\n')) {
                if (!token.empty() && (token[token.size() - 1] == '\n' || token[token.size() - 1] == '\r')) {
                    token.erase(token.size() - 1);
                }
                result.push_back(token);
            }
        }

        return result;
    }

    bool sendCommand(const char* command) {
        if (write(master_fd, command, std::strlen(command)) != std::strlen(command)) {
            std::cerr << "write failed\n";
            return false;
        }

        return true;
    }

	int parsePermissions(const std::string& perm_string) {
	    int owner = 0, group = 0, others = 0;

	    if (perm_string[1] == 'r') owner += 4;
	    if (perm_string[2] == 'w') owner += 2;
	    if (perm_string[3] == 'x') owner += 1;

	    if (perm_string[4] == 'r') group += 4;
	    if (perm_string[5] == 'w') group += 2;
	    if (perm_string[6] == 'x') group += 1;

	    if (perm_string[7] == 'r') others += 4;
	    if (perm_string[8] == 'w') others += 2;
	    if (perm_string[9] == 'x') others += 1;

	    return owner * 100 + group * 10 + others;
	}

	std::tm parseDateTime(const std::string& date, const std::string& timeOrYear) {
	    std::tm tm = {};
	    std::istringstream date_stream(date);

	    if (timeOrYear.find(":") != std::string::npos) {
	        // Если вместо года указано время
	        date_stream >> std::get_time(&tm, "%b %d");

	        // Используем текущий год
	        std::time_t t = std::time(NULL);
	        std::tm* current_time = std::localtime(&t);
	        tm.tm_year = current_time->tm_year;
	        
	        // Парсим время
	        std::istringstream time_stream(timeOrYear);
	        time_stream >> std::get_time(&tm, "%H:%M");
	    } else {
	        // Если указан год
	        date_stream >> std::get_time(&tm, "%b %d");
	        
	        // Парсим год
	        std::istringstream year_stream(timeOrYear);
	        year_stream >> tm.tm_year;
	        tm.tm_year -= 1900; // std::tm использует годы, считая от 1900 года
	    }

	    return tm;
	}

	FileInfo parseFileLine(const std::string& line) {
	    FileInfo info;

	    // Разбиваем строку на токены
	    std::istringstream iss(line);
	    std::vector<std::string> tokens((std::istream_iterator<std::string>(iss)),
	                                     std::istream_iterator<std::string>());

		if (tokens.size() < 9) {
		    //std::cerr << "Parsing error: insufficient number of tokens in line \"" << line << "\"\n";
		    return FileInfo();
		}

	    // Парсим и заполняем структуру FileInfo
	    info.permissions = parsePermissions(tokens[0]);
	    info.owner = tokens[2];
	    info.group = tokens[3];
	    info.size = std::stol(tokens[4]);

	    // Парсим время последнего изменения. Здесь предполагается, что формат даты - это "MMM dd yyyy",
	    // где MMM - трехбуквенное английское сокращение месяца, dd - двухзначное число дня, yyyy - четырехзначное число года.
	    // Учитывается, что вместо года может быть время изменения.
		//std::tm tm = parseDateTime(tokens[5] + " " + tokens[6], tokens[7]);
		//auto system_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
		//info.modified_time = std::filesystem::file_time_type(std::chrono::duration_cast<std::filesystem::file_time_type::duration>(system_time.time_since_epoch()));
		std::tm tm = parseDateTime(tokens[5] + " " + tokens[6], tokens[7]);
		auto system_time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
		info.modified_time = std::filesystem::file_time_type(std::chrono::duration_cast<std::filesystem::file_time_type::duration>(system_time.time_since_epoch()));
	    
	    info.path = tokens[8];

	    info.is_directory = (tokens[0][0] == 'd');

	    return info;
	}

    std::vector<FileInfo> ls() {
        sendCommand("ls -l\n");
        std::vector<std::string> lines_tmp = waitFor("sftp>", {});
        std::vector<std::string> lines;
        bool secondWait = false;
	    for(const auto& str : lines_tmp) {
			if ((str.rfind("sftp>", 0) == 0) && (str.length() > 6)) {
				// it's "sftp> ls -l", let's wait for next "sftp>"
				secondWait = true;
			}
	        //std::cout << str.length() << " ^ " << str << std::endl;
	    }
		if (secondWait) {
	        lines = waitFor("sftp>", {});
		} else {
			lines = lines_tmp;
		}

        std::vector<FileInfo> files;

		// Записываем в список файлов ".."
		FileInfo i = FileInfo();
		i.path = "..";
		i.is_directory = true;
		files.push_back(i);

        for (const auto& line : lines) {
            if (!line.empty()) {
                FileInfo fi = parseFileLine(line);
                if (!fi.owner.empty()) {
	                files.push_back(fi);
				}
            }
        }

        return files;
    }

	bool cd(const std::string& directory) {
	    std::string command = "cd \"" + directory + "\"\n";
	    if (!sendCommand(command.c_str())) {
	        return false;
	    }
	    // Проверим, что команда выполнена успешно, ожидая приглашения "sftp>"
	    return !waitFor("sftp>", {}).empty();
	}

	bool mkdir(const std::string& directory) {
	    std::string command = "mkdir \"" + directory + "\"\n";
	    if (!sendCommand(command.c_str())) {
	        return false;
	    }
	    // Проверим, что команда выполнена успешно, ожидая приглашения "sftp>"
	    return !waitFor("sftp>", {}).empty();
	}

	bool rmdir(const std::string& directory) {
	    std::string command = "rmdir \"" + directory + "\"\n";
	    if (!sendCommand(command.c_str())) {
	        return false;
	    }
	    // Проверим, что команда выполнена успешно, ожидая приглашения "sftp>"
	    return !waitFor("sftp>", {}).empty();
	}

	bool rm(const std::string& filename) {
	    std::string command = "rm \"" + filename + "\"\n";
	    if (!sendCommand(command.c_str())) {
	        return false;
	    }
	    // Проверим, что команда выполнена успешно, ожидая приглашения "sftp>"
	    return !waitFor("sftp>", {}).empty();
	}

    ~SftpClient() {
       if (pid > 0) {
            // Проверяем, завершен ли дочерний процесс
            int status;
            if (waitpid(pid, &status, WNOHANG) == 0) {
                // Если нет, отправляем сигнал для завершения
                kill(pid, SIGTERM);
                waitpid(pid, &status, 0); // Ожидаем завершения
            }
            std::cout << "Child exited with status " << status << '\n';
        }
    }
};

/*
void doLs(std::vector<FileInfo> files) {

    for (const auto& file : files) {

		auto epoch_time = std::chrono::duration_cast<std::chrono::seconds>(file.modified_time.time_since_epoch());
		std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(epoch_time));
		std::string time_string = std::ctime(&tt);
		time_string.pop_back();

        std::cout << "Test tool: [BEGIN]"
        	<< " # " << file.permissions
        	<< " # " << std::setw(8) << std::setfill(' ') << file.owner
        	<< " # " << std::setw(8) << std::setfill(' ') << file.group
        	<< " # " << time_string
        	<< " # " << std::setw(12) << std::setfill(' ') << file.size
        	<< " # " << std::setw(24) << std::setfill(' ') << file.path
        	<< " # "
        	<< "[END]" << std::endl;

        //std::cout << "File: " << file.path << ", Owner: " << file.owner << ", Size: " << file.size << " bytes\n";
    }
}
*/