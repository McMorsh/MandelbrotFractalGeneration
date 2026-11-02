# MandelbrotFractalGeneration

[![Build](https://github.com/McMorsh/MandelbrotFractalGeneration/actions/workflows/build.yml/badge.svg)](https://github.com/McMorsh/MandelbrotFractalGeneration/actions/workflows/build.yml)

Визуализация фракталов на C++20: множество Мандельброта и множество Жюлиа.
Пиксели считаются параллельно через OpenMP, изображение выводится с помощью
raylib.

![Общий вид множества Мандельброта](docs/preview.png)

## Возможности

- сглаженная раскраска на основе непрерывного числа итераций;
- два фрактала: множество Мандельброта и множество Жюлиа, переключение по T;
- три палитры (Ember, Ocean, Mono) с переключением по клавише и запоминанием выбора;
- приближение колесом мыши к точке под курсором и панорамирование;
- панорамирование стрелками и масштабирование клавишами `+` и `-`;
- копирование центра, масштаба, итераций и палитры в буфер обмена по C
  и вставка обратно по V;
- автоматический рост числа итераций при приближении и ручная подстройка;
- параллельный расчёт строк изображения через OpenMP;
- фоновый расчёт кадра: интерфейс не блокируется на глубоком зуме;
- изменение размера окна с пересчётом кадра под новый размер;
- предпросмотр в половинном разрешении при перетаскивании, зуме и изменении
  размера окна, полный кадр после остановки;
- сохранение PNG без интерфейса в разрешении FHD, 2K, 4K или 8K по F2;
- выбор разрешения PNG по Shift + влево / вправо;
- иконка окна и exe из `assets/icon.png` и `assets/icon.ico`;
- русский и английский интерфейс на шрифте IBM Plex Mono с переключением по L;
  выбранный язык сохраняется между запусками;
- рендер PNG без окна с параметрами командной строки;
- пересчёт изображения только при изменении вида, а не каждый кадр.

Пример приближения к границе множества:

![Приближение к границе множества](docs/zoom.png)

## Управление

| Действие | Клавиши и мышь |
| --- | --- |
| Приблизить или отдалить | Колесо мыши, `+` и `-` |
| Переместить изображение | Стрелки, левая кнопка мыши и перетаскивание |
| Скопировать или вставить вид | C и V |
| Увеличить или уменьшить число итераций | `[` и `]` |
| Сменить палитру | P |
| Сменить фрактал | T |
| Сбросить вид | R |
| Сохранить PNG в выбранном разрешении | F2 |
| Выбрать разрешение PNG | Shift + влево / вправо |
| Показать или скрыть подсказки | H |
| Переключить язык интерфейса | L |
| Выход | Esc |

Клавиша T переключает множество Мандельброта и множество Жюлиа. У Жюлиа
параметр `c` меняется двумя ползунками в панели `Julia c` справа внизу: `Re` и
`Im`, диапазон -2...2. Значение по умолчанию `-0.8 + 0.156i`, вид по умолчанию
центрирован в нуле с шириной 4, клавиша R возвращает к нему. Выбранный тип и `c`
сохраняются между запусками. Смена типа сохраняет текущий вид, поэтому после
переключения может понадобиться R. В окне уже 800 px панель подсказок
скрывается, чтобы не перекрывать ползунки.

Сохранённое изображение не содержит панелей и подсказок. F2 считает кадр в
выбранном разрешении в фоне и сохраняет `mandelbrot_ГГГГММДД_ЧЧММСС_4k.png`.
Пресеты: FHD (1920x1080), 2K (2560x1440), 4K (3840x2160) и 8K (7680x4320).
Текущий виден в HUD как `PNG 4K` и переключается по Shift + влево / вправо.
Выбор запоминается и восстанавливается при следующем запуске.
Кадр сохраняет вертикальный охват окна и расширяется по горизонтали до 16:9.
Окно продолжает отвечать, а в строке состояния показывается `PNG 8K`. На 8K
расчёт занимает заметно больше времени и требует больше памяти.

F12 обрабатывает сама raylib: она сохраняет `screenshotNNN.png` вместе с
интерфейсом. Поэтому для чистого PNG используется отдельная клавиша.

Клавиша C копирует в буфер обмена центр, span, зум, число итераций и палитру.
Клавиша V вставляет эти параметры обратно; зум восстанавливается из span.
Если в буфере нет подходящего текста или параметры несовместимы, вид не
меняется и появляется сообщение об ошибке. Удерживайте `[` или `]`, чтобы
менять число итераций подряд. Шаг равен 100, диапазон 50...5000; на границе
нажатия игнорируются, поэтому обратный шаг срабатывает сразу.

Клавиша L переключает русский и английский интерфейс. Заголовок окна, HUD,
панель подсказок и уведомления обновляются сразу. Выбранный язык сохраняется в
`settings.txt`: Windows - `%APPDATA%\MandelbrotFractalGeneration\settings.txt`,
Linux - `$XDG_CONFIG_HOME/MandelbrotFractalGeneration/settings.txt` или
`~/.config/MandelbrotFractalGeneration/settings.txt`. Там же хранятся выбранная
палитра, пресет PNG, тип фрактала и параметр `c` для Julia. Если файл удалить,
при следующем запуске снова будет русский язык, Ember, FHD и Мандельброт.

Пример файла:

```text
language=ru
palette=Ember
fractal=julia
julia-re=-0.8
julia-im=0.156
png=4K
```

Окно можно потянуть за край или угол, минимальный размер 640x480. Фрактал
пересчитывается под новый размер, центр и масштаб сохраняются.

## Рендер без окна

Без `--output` запускается интерактивное окно. С `--output` приложение
рендерит PNG и сразу завершается:

```bash
./build-mingw/MandelbrotFractalGeneration.exe \
  --output mandelbrot.png \
  --width 1920 --height 1080 \
  --iterations 800 \
  --center -0.743643887037151 0.131825904205330 \
  --span 0.001 \
  --palette Ocean
```

| Опция | Значение |
| --- | --- |
| `-o, --output <file>` | путь к PNG, включает режим без окна |
| `--width <px>` | ширина изображения, по умолчанию 1200 |
| `--height <px>` | высота, по умолчанию 800 |
| `--iterations <n>` | число итераций, по умолчанию подбирается по `--span` |
| `--center <re> <im>` | центр окна, по умолчанию `-0.6 0` |
| `--span <value>` | ширина окна в комплексных единицах, по умолчанию 3.5 |
| `--palette <name>` | `Ember`, `Ocean` или `Mono` |
| `--fractal <name>` | `mandelbrot` или `julia`, по умолчанию `mandelbrot` |
| `-h, --help` | справка |

Имена палитр не зависят от регистра. Остальные опции относятся только к
режиму без окна. В GUI параметр `c` у Julia меняется ползунками, а headless
использует значение по умолчанию `-0.8 + 0.156i`.

## Как это работает

Для каждой точки c на комплексной плоскости вычисляется последовательность

```text
z₀ = 0
zₙ₊₁ = zₙ² + c
```

Итерации продолжаются, пока |zₙ| ≤ 2. Выход за этот предел означает, что точка
лежит вне множества, а число итераций до выхода задаёт цвет. У Мандельброта
`z₀ = 0`, а точкой плоскости является `c`. У Жюлиа наоборот: точка плоскости
задаёт `z₀`, а `c` постоянно и равно `-0.8 + 0.156i`, поэтому для каждого `c`
получается свой узор. Чтобы убрать ступени между соседними цветами,
используется сглаженная оценка:

```text
μ = n + 1 − log₂(log₂ |zₙ|)
```

Точки, не вышедшие за предел за `maxIterations` шагов, считаются внутренними
и окрашиваются в чёрный.
Палитра циклическая: цвет плавно переходит между опорными цветами и
зацикливается. Доступны три схемы: Ember, Ocean и Mono. Длина цветового цикла
растёт вместе с `maxIterations` (но не меньше 20 итераций), поэтому при
глубоком приближении полосы не становятся слишком частыми. Выбранная схема
сохраняется в настройках и восстанавливается при следующем запуске.

## Сборка

Требования:

- компилятор с поддержкой C++20;
- CMake 3.21 или новее;
- OpenMP;
- raylib 5.x или новее;
- GoogleTest (для тестов).

### Windows (MSYS2 / MinGW)

Установите [MSYS2](https://www.msys2.org/) в `C:\msys64` и откройте терминал
**MSYS2 UCRT64**:

```bash
pacman -Syu
pacman -S --needed git mingw-w64-ucrt-x86_64-gcc \
  mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-libgomp \
  mingw-w64-ucrt-x86_64-gtest
```

Сборка:

```bash
git clone https://github.com/McMorsh/MandelbrotFractalGeneration.git
cd MandelbrotFractalGeneration

cmake --preset mingw-release
cmake --build --preset mingw-release
./build-mingw/MandelbrotFractalGeneration.exe
```

Чтобы запускать exe из PowerShell или Проводника, не нужно вручную менять PATH:
CMake при сборке через MinGW сам копирует нужные DLL рядом с exe. Иконка окна
`assets/icon.png`, шрифт `assets/IBMPlexMono-Regular.ttf` и лицензия
`assets/OFL.txt` копируются в `assets/` рядом с exe, а `assets/icon.ico`
встраивается в сам файл.

### Linux

```bash
git clone https://github.com/McMorsh/MandelbrotFractalGeneration.git
cd MandelbrotFractalGeneration

# Зависимости raylib для Debian и Ubuntu
sudo apt update
sudo apt install build-essential cmake git curl zip unzip tar pkg-config \
  libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev \
  libxinerama-dev libasound2-dev libwayland-dev libxkbcommon-dev wayland-protocols

git clone https://github.com/microsoft/vcpkg "$HOME/vcpkg"
"$HOME/vcpkg/bootstrap-vcpkg.sh"
"$HOME/vcpkg/vcpkg" install raylib gtest

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j

./build/MandelbrotFractalGeneration
```

## Тесты

```bash
cmake --preset mingw-release
cmake --build --preset mingw-release
ctest --preset mingw-release
```

На Linux тесты запускаются через каталог сборки:

```bash
ctest --test-dir build --output-on-failure
```

Если тесты не нужны, конфигурируй с `-DBUILD_TESTING=OFF`.

## Структура проекта

| Путь | Назначение |
| --- | --- |
| `FractalCore/Fractal.h`, `FractalCore/Fractal.cpp` | Расчёт выбранного фрактала и сглаженных итераций |
| `FractalCore/FractalType.h`, `FractalCore/FractalType.cpp` | Тип фрактала, параметры Julia и разбор имени |
| `FractalCore/View.h`, `FractalCore/View.cpp` | Камера: зум, панорамирование и координаты |
| `FractalCore/Iterations.h`, `FractalCore/Iterations.cpp` | Автоматическое число итераций и ручная подстройка |
| `FractalCore/ViewSnapshot.h`, `FractalCore/ViewSnapshot.cpp` | Формат и разбор параметров вида для буфера обмена |
| `ColorMap/ColorMap.h`, `ColorMap/ColorMap.cpp` | Палитры и преобразование итераций в цвет |
| `Renderer/ExportPresets.h` | Пресеты разрешения FHD, 2K, 4K и 8K |
| `Renderer/FractalRender.h`, `Renderer/FractalRender.cpp` | Окно, текстура, интерфейс и экспорт PNG |
| `Renderer/ImageRender.h`, `Renderer/ImageRender.cpp` | Рендер и сохранение PNG без окна |
| `Renderer/ImageExportWorker.h`, `Renderer/ImageExportWorker.cpp` | Фоновый экспорт PNG в выбранном разрешении |
| `Renderer/Localization.h`, `Renderer/Localization.cpp` | Строки русского и английского интерфейса |
| `Renderer/RenderWorker.h`, `Renderer/RenderWorker.cpp` | Фоновый расчёт кадров |
| `Renderer/Settings.h`, `Renderer/Settings.cpp` | Сохранение и чтение настроек интерфейса |
| `main.cpp` | CLI, обработка ввода и главный цикл |
| `tests/` | Тесты GoogleTest для `Fractal`, `FractalType`, `Iterations`, `ColorMap`, `View`, `ViewSnapshot`, `ImageRender`, `ExportPresets`, `Localization` и `Settings` |
| `assets/` | Иконки, шрифт IBM Plex Mono и текст лицензии OFL |
| `docs/` | Изображения для README |

## Будущие улучшения

- Burning Ship и другие фракталы;
- кэш последних кадров для возврата к уже посещённому виду;
- рендеринг на GPU;
- готовые сборки в GitHub Releases.

## Лицензия

Проект распространяется по лицензии [MIT](LICENSE).

Шрифт IBM Plex Mono распространяется по SIL Open Font License 1.1.
Текст лицензии находится в `assets/OFL.txt`.
