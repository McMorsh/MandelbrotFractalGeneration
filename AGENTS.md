# AGENTS.md

Правила для людей и AI-агентов, которые меняют этот репозиторий.
Цель: минимальные, предсказуемые и проверяемые изменения.

## Базовый стандарт

Основываемся на [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
Если правило ниже не описывает случай, действует Google C++ Style Guide.
Документация проекта (`README.md`, `AGENTS.md`, CMake) остаётся на русском
языке, код и комментарии в коде пишутся на английском.

Проект использует C++20, CMake, OpenMP и raylib. Целевые платформы: Windows
(MSYS2 UCRT64) и Linux.

## Форматирование

- Источник правил: `.clang-format` на основе Google. Отступ 2 пробела, ширина
  строки 80, табы запрещены.
- Содержимое namespace не сдвигается (`NamespaceIndentation: None`).
- Спецификаторы доступа `public`, `protected` и `private` сдвигаются на один
  пробел.
- Открывающая скобка на той же строке.
- Одна инструкция на строку.
- Пробел после ключевых слов (`if`, `for`, `while`, `switch`), вокруг бинарных
  операторов и после запятых.
- Указатели и ссылки: `Type* name`, `Type& name`.
- `nullptr` вместо `NULL` и `0`.
- `const` и `constexpr` везде, где это возможно.
- Для размеров и индексов используйте `std::size_t`, типы приводите явно через
  `static_cast`.
- Переносы строк, короткие функции и пустые блоки форматирует `clang-format`.
  Не выравнивайте код вручную.
- Предпочитайте короткие функции. Если функция длиннее примерно 40 строк,
  подумайте, можно ли разделить её без потери читаемости.
- Именованные константы вместо магических чисел.
- Используйте C++-заголовки (`<cmath>`, `<cstdio>`), а не C-заголовки
  (`<math.h>`, `<stdio.h>`).
- Для размера конкретной переменной используйте `sizeof(variable)`, а не
  `sizeof(Type)`.

### Порядок include

1. Собственный заголовок `.hpp`.
2. Заголовки стандартной библиотеки C++.
3. Сторонние библиотеки, например `<raylib.h>`.
4. Заголовки проекта.

Внутри каждой группы сортируйте по алфавиту. Не полагайтесь на транзитивные
include: подключайте то, что используете. Для C++ используйте исходники `.cpp`
и заголовки `.hpp`, для C - `.c` и `.h`. Не смешивайте расширения в одном
проекте.

### Заголовки

- Include guard в формате `FRACTALGENERATION_<PATH>_<FILE>_HPP_`.
  `#pragma once` не используем.
- В заголовке только объявления и короткие inline-функции. Тяжёлую логику
  держите в `.cpp`.
- Файлы и каталоги называйте строчными буквами в snake_case, без пробелов и
  русских символов.

## Имена

| Сущность | Стиль | Пример |
| --- | --- | --- |
| Классы, struct, enum class | PascalCase | `FractalRender`, `View` |
| Функции и методы | PascalCase, с глаголом | `Compute`, `SaveImage`, `GetWidth` |
| Локальные переменные и параметры | snake_case | `max_iter`, `center_x` |
| Поля классов | snake_case + `_` | `max_iterations_`, `colors_` |
| Поля struct | snake_case | `render_ms`, `show_help` |
| Константы и перечислители | `k` + PascalCase | `kWindowWidth`, `kMandelbrot` |
| Файлы модулей | snake_case, строчные буквы | `fractal.cpp`, `color_map.hpp` |
| Тестовые файлы | snake_case, суффикс `_test` | `fractal_test.cpp` |
| Тестовые suite и case | PascalCase | `TEST(FractalTest, PointInsideSet...)` |

Имена на английском, без транслитерации и лишних сокращений. Общепринятые
сокращения допустимы: `iter`, `info`, `max`. Аббревиатуры пишите как обычные
слова: `GetItmoYear`, `PrintSfinaeRules`. Функция обычно начинается с глагола:
`Get`, `Set`, `Compute`, `Parse`, `Save`, `Handle`. Булевы переменные называйте
так, чтобы имя читалось как условие: `show_help`, `dirty`, `has_texture`.
Все перечисления объявляйте через `enum class`.

## Классы и владение ресурсами

- Данные держите приватными. Секция `public` идёт перед `private`.
- Однопараметрические конструкторы помечайте `explicit`.
- Методы, не меняющие состояние, делайте `const`.
- Владеющий ресурсом класс освобождает его в деструкторе. Копирование запрещайте
  через `= delete`, если копия привела бы к двойному освобождению. Для raylib это
  `Image`, `Texture2D` и окно.
- Не используйте `new`, `delete` и владеющие сырые указатели. Для массивов и строк
  используйте `std::vector` и `std::string`.
- Не используйте `using namespace std;`. Пишите `std::` явно.
- Заголовки не должны вводить `using namespace`. В `.cpp` локальный `using`
  допустим, если он сокращает длинный тип и не влияет на заголовки.
- Внутренние функции и константы файла держите в анонимном namespace.

## Ошибки и OpenMP

- Исключения не используем. Ошибку возвращайте через `bool`, пустой контейнер или
  код ошибки. Для ошибок программиста допустим `assert`.
- Проверяйте входные данные на границах модуля: размеры больше нуля, `span > 0`,
  `max_iterations > 0`.
- `#pragma omp parallel for` ставьте только над циклом, итерации которого
  независимы. Не пишите в общие переменные без синхронизации.
- Для равномерной нагрузки используйте `schedule(static)`, для строк фрактала с
  разной стоимостью `schedule(dynamic)`.
- Результат не должен зависеть от числа потоков.

## Комментарии

Пишем по-английски, коротко и по делу. Комментарий объясняет причину,
ограничение или неочевидную математику, а не пересказывает код.

Правила:

- Обычные комментарии: `//`. Блочные `/* ... */` не используем.
- Публичный API в заголовках документируйте Doxygen-комментариями `///`.
  `/** ... */` не используем.
- Первая строка `///` - краткое описание. `@brief` не нужен.
- `@param` добавляйте, только если имя параметра не объясняет его смысл,
  единицы, диапазон или направление (входной или выходной).
- `@return` добавляйте, если возвращаемое значение требует пояснения.
- Комментарий к публичному API ставится в заголовке над объявлением: назначение,
  параметры, возвращаемое значение, предусловия.
- В `.cpp` комментируйте только неочевидные места: формулу, порядок операций,
  причину оптимизации.
- Не оставляйте закомментированный код. Для истории есть Git.
- Не используйте декоративные разделители, эмодзи, маркетинговые прилагательные
  и шаблонные фразы.
- Не используйте длинные тире U+2014 и U+2013. Только дефис или перестроенная
  фраза.
- TODO: `// TODO(McMorsh): ...`.
- Закрывающий namespace: `}  // namespace`.
- Устаревший комментарий считается ошибкой. Меняете поведение, меняйте
  комментарий.

Примеры:

```cpp
// Bad: restates the code.
// Increment the iteration count.
iterations++;

// Good: explains why the square root is not needed.
// std::norm(z) returns |z|^2, so the square root is unnecessary.
while (std::norm(z) <= 4.0 && iterations < max_iterations) {
  z = z * z + c;
  iterations++;
}
```

```cpp
/// Color of a pixel for a smoothed iteration count.
/// Points with iter >= max_iter are treated as interior.
Color GetColor(double iter, int max_iter) const;

/// Scales the span by factor while keeping the point under the cursor.
/// @param factor positive multiplier
/// @return false if the scale did not change or factor is not positive.
bool ZoomAt(double factor, double screen_x, double screen_y);
```

```cpp
/// Computes the selected fractal for a window of the complex plane.
class Fractal {
 public:
  /// Smoothed iteration count for the point real + i * imag.
  /// @return max_iterations if the point did not escape |z| = 2.
  double Compute(double real, double imag) const;
};
```

## Тесты

Тесты лежат в `tests/`, собираются при `BUILD_TESTING` и запускаются через
CTest.

Правила:

- Файлы: `tests/<модуль>_test.cpp`.
- Один тест проверяет одно поведение. Имя описывает ожидаемый результат:
  `PointInsideSetReturnsIterationLimit`.
- Используйте `EXPECT_*`, если после ошибки тест может продолжаться, и `ASSERT_*`,
  если продолжение бессмысленно.
- Дробные числа сравнивайте через `EXPECT_DOUBLE_EQ` или `EXPECT_NEAR` с явным
  допуском. Не сравнивайте `double` через `==`.
- Тесты детерминированы: без `sleep`, без зависимости от времени, порядка потоков,
  состояния окна и реального GPU.
- Не тестируйте приватные детали. Проверяйте публичный контракт.
- Для OpenMP прогоняйте тесты с `OMP_NUM_THREADS=1` и дополнительно с несколькими
  потоками, если результат может зависеть от параллельности.
- GUI и экспорт PNG не тестируйте с реальным окном. Для `SaveIterationImage`
  достаточно проверить, что файл создан и начинается с сигнатуры PNG. Пиксельный
  эталон хрупок.
- Камеру (`View`) держите в `fractal_core/view.hpp`, она покрыта
  `tests/view_test.cpp`. Размеры экрана передавайте в `View` снаружи, чтобы зум и
  панорамирование проверялись без окна.

Пример:

```cpp
#include <gtest/gtest.h>

#include "fractal.hpp"

namespace {

TEST(FractalTest, PointInsideSetReturnsIterationLimit) {
  const Fractal fractal(200);

  EXPECT_DOUBLE_EQ(fractal.Compute(0.0, 0.0), 200.0);
}

TEST(FractalTest, PointOutsideSetEscapesQuickly) {
  const Fractal fractal(200);

  EXPECT_LT(fractal.Compute(3.0, 0.0), 5.0);
}

}  // namespace
```

Подключение в CMake:

```cmake
include(CTest)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)

    add_executable(fractal_tests
            tests/fractal_test.cpp
            tests/color_map_test.cpp
            fractal_core/fractal.cpp
            color_map/color_map.cpp)

    target_link_libraries(fractal_tests PRIVATE
            GTest::gtest_main
            raylib
            OpenMP::OpenMP_CXX)

    include(GoogleTest)
    gtest_discover_tests(fractal_tests)
endif()
```

Запуск:

```bash
cmake -B build-mingw -S . -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build-mingw
ctest --test-dir build-mingw --output-on-failure
```

## CMake

- Только target-based команды: `target_include_directories`,
  `target_link_libraries`, `target_compile_options`. Не используйте глобальные
  `include_directories` и `link_directories`.
- Новые `.cpp` добавляйте в `add_executable` явно.
- Зависимости подключайте через `find_package` и imported targets.
- Не хардкодьте абсолютные пути. Используйте `CMAKE_CURRENT_SOURCE_DIR` и
  generator expressions.
- Предупреждения остаются включёнными: `/W4` для MSVC, `-Wall -Wextra -Wpedantic`
  для GCC и Clang.
- Тесты включайте только при `BUILD_TESTING`.

## Сборка и проверка

Windows, терминал MSYS2 UCRT64:

```bash
cmake -B build-mingw -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-mingw
./build-mingw/FractalGeneration.exe
```

Linux:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j
./build/FractalGeneration
```

Перед завершением задачи:

1. Соберите проект без новых предупреждений.
2. Запустите `ctest`, если тесты есть.
3. Проверьте, что приложение запускается и не падает.
4. Просмотрите `git diff`: в нём не должно быть отладочного вывода, мусорных
   файлов и случайных переименований.

## Git

- Формат коммитов: Conventional Commits, английский императив, до 50 символов в
  заголовке, тело до 72 символов.
- Один коммит - одно логическое изменение. Не смешивайте фичу, рефакторинг и
  форматирование.
- Не добавляйте в репозиторий `build*`, `vcpkg`, сгенерированные скриншоты,
  `.clangd`, личные заметки и секреты.
- Агент не коммитит, не пушит, не делает rebase и не меняет историю без явной
  просьбы.
- Перед push синхронизируйтесь с `origin/main`. Если ветки разошлись, используйте
  rebase, если пользователь не попросил обратное.

## Чек-лист агента

- Прочитайте соседний код и следуйте его стилю.
- Делайте минимальный дифф по задаче.
- Не добавляйте зависимости без необходимости.
- Не переименовывайте файлы и API без явной просьбы.
- Не оставляйте комментарии, которые пересказывают код.
- Не используйте длинные тире, эмодзи и шаблонные фразы.
- Соберите проект и запустите тесты.
- Если требование неоднозначно, задайте вопрос до правки.
