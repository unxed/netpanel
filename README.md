# netpanel

**Readme in english is below**

Сетевой плагин для far2l (на данный момент есть поддержка только sftp). Работает как обертка над sftp-клиентом командной строки, так что никаких зависимостей не требуется.

Что работает?

- подключение с помощью файла ключа
- подключение по паролю
- добавление хоста в список известных хостов
- просмотр файлов и папок
- загрузка файлов и папок с сервера
- выгрузка файлов и папок на сервер
- создание папок
- удаление файлов и пустых папок
- протестировано как минимум на Ubuntu 22.04

Что пока не работает (но планируется в будущем)?

- индикация хода загрузки/выгрузки
- просмотр, редактирование файлов на сервере
- поддержка выбора нескольких файлов/папок (в настоящее время плагин способен работать только с одним файлом/папкой под курсором)
- удаление непустых папок
- поддержка других подобных приложений командной строки (например, ftp)
- на некоторых роутерах нет sftp, но есть ssh и scp. хорошо бы поддерживать и работу через них тоже
- справка
- языки/переводы

Плагин сделан на основе TmpPanel из состава far2l и все еще содержит некоторый неиспользуемый код оттуда.

Для сборки клонируйте этот репозиторий в корневую папку дерева исходников far2l, и добавьте
```
add_subdirectory (netpanel)
```
в конец корневого файла CMakeLists.txt

Потребуется компилятор, поддерживающий C++17 (gcc 7+, clang 5+).

См. также: https://github.com/elfmz/far2l/issues/1819

---

network plugin for far2l (currently sftp only). Works as a wrapper against command line sftp tool, so no dependencies needed.

That works?

- connecting by key file
- connecting by password
- adding host to known hosts list
- browsing files and folders
- downloading from server files and folder
- uploading to server files and folder
- making folders
- removing files and empty folder
- tested at least on Ubuntu 22.04

That still does not (but planned in future)?

- download/upload progress indication
- viewing, editing remote files
- selection support (currently plugin is capable working with one file/folder under a cursor only)
- removing non empty folders
- support for the other similar tools (like ftp)
- some routers lacks sftp tool, but have ssh and scp. if possible, we should support using those instead of sftp
- help
- languages/translations

Heavily based on TmpPanel far2l plugin and still contains some unused code from it.

To build, clone this repo inside the root of far2l sources tree, and add
```
add_subdirectory (netpanel)
```
to the end of root CMakeLists.txt

C++17 supporting compiler is requred (gcc 7+, clang 5+).

See also: https://github.com/elfmz/far2l/issues/1819
