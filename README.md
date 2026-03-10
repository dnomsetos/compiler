# compiler
В этом репозитории будет находиться реализация простого компилятора, грамматика описана в файле `grammar.txt`.
## Сборка и запуск тестов
В корне проекта:
```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-13 -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target tun_tests
```

