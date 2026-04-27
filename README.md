# TrayApp

Windows GUI-приложение с иконкой в области уведомлений.

## Возможности

- иконка в трее после запуска;
- левый клик по иконке открывает главное окно;
- правый клик открывает контекстное меню;
- контекстное меню содержит команды `Открыть` и `Выход`;
- приложение не допускает запуск второго экземпляра через named mutex;
- иконка восстанавливается после пересоздания панели задач;
- сборка через CMake и Visual Studio 2022.

## Сборка

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A ARM64
cmake --build build --config Release
```

Для x64 используйте:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Готовый файл находится здесь:

```text
build\bin\TrayApp.exe
```

Проект собирается со статическим MSVC runtime, поэтому `TrayApp.exe` не должен требовать отдельной установки Visual C++ Redistributable.

## Запуск

Запустите `TrayApp.exe`. Приложение добавит иконку в трей и продолжит работать, пока не будет выбрана команда `Выход` в контекстном меню.

## Структура

```text
.
├── .github/workflows/build.yml
├── CMakeLists.txt
├── README.md
└── src/
    └── TrayApp/
        ├── CMakeLists.txt
        ├── main.cpp
        ├── MainWindow.h
        ├── MainWindow.cpp
        ├── pch.h
        └── pch.cpp
```

## CI

GitHub Actions собирает только GUI-приложение `TrayApp.exe` при push в любую ветку и при pull request в `main`.
