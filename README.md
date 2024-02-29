# **Transport Catalogue**

## **Описание**

Transport Catalogue - проект, который представляет собой транспортный справочник и обладает следующими функциями:

* Хранит и предоставляет информацию о заданных остановках и маршрутах
* Выводит карту с остановками и маршрутами в формате SVG
* Прокладывает кратчайший маршрут между заданными остановками

Приём запросов и выдача ответов осуществляется в формате JSON. Классы форматов JSON и SVG были разработаны самостоятельно
Так как транспортный справочник является обновляемой базой данных, то взаимодействие с ним разделено на две стадии - создание этой самой базы и получение ответов на запросы. В проекте реализована бинарная сериализация и десериализация данных с помощью Google Protobuf

## **Сборка проекта**
Необходимо скачать и установить [Google Protobuf](https://github.com/protocolbuffers/protobuf).
В местоположении проекта нужно создать папку, где будет собираться проект, например, build.
В новой папке собираем Transport Catalogue с помощью следующих команд в консоли bash:
```
cmake . -DCMAKE_PREFIX_PATH="тут нужно указать путь до protoc.exe"
cmake --build .
```
## **Работа с проектом**
Взаимодействие с проектом разделено на две стадии. Такой подход необходим для решения проблемы с производительностью: построение графов для просчёта маршрутов - это длительный процесс, поэтому он осуществляется только на этапе создания базы. При обработке запросов происходит работа с уже готовым графом, и заново вычисления производить не нужно. Сериализация с использованием Google Protobuf помогает оптимизировать две задачи - хранение большой базы данных и передача по сети

Для того, чтобы создать базу транспортного справочника, нужно поместить в папку с transport_catalogue.exe файл расширения JSON с названием make_base, в котором содержатся запросы base_requests. Затем запустить программу с параметром make_base:

```
transport_catalogue.exe make_base
```
После создания базы можно приступать к работе с ней. Для этого необходимо аналогично make_base.json создать файл process_requests.json, после чего запустить программу с параметром process_requests. Ответы на запросы запишутся в файл result.json.

```
transport_catalogue.exe process_requests
```
Сериализация и десериализация данных происходит автоматически.

### **Формат входных данных**

Входные данные поступают программе из потока ввода в формате JSON-объекта, который имеет на верхнем уровне следующую структуру:

```
{
  "base_requests": [ ... ],
  "render_settings": { ... },
  "routing_settings": { ... },
  "serialization_settings": { ... },
  "stat_requests": [ ... ]
}
```

Это словарь, содержащий ключи:
`base_requests` — массив с описанием автобусных маршрутов и остановок.
`stat_requests` — массив с запросами к транспортному справочнику.
`render_settings` — словарь для отрисовки изображения.
`routing_settings` — словарь, содержащий в себе настройки для скорости автобусов и времени ожидания на остановке.
`serialization_settings` — настройки сериализации.

### **Заполнение базы транспортного справочника**

**Сериализация базы данных**

В ключе file указывается название файла, в который будет сериализована база.

```
"serialization_settings": {
    "file": "transport_catalogue.db"
}
```

**Пример описания остановки:**

```
{
  "type": "Stop",
  "name": "Электросети",
  "latitude": 43.598701,
  "longitude": 39.730623,
  "road_distances": {
    "Улица Докучаева": 3000,
    "Улица Лизы Чайкиной": 4300
  }
}
```

Описание остановки — словарь с ключами:
`type` — строка, равная "Stop", означает, что словарь описывает остановку;
`name` — название остановки;
`latitude` и `longitude` - широта и долгота остановки;
`road_distances` — словарь, задающий расстояние до соседних остановок. Ключ — название остановки, значение — целое число в метрах.

**Пример описания автобусного маршрута:**

```
{
  "type": "Bus",
  "name": "14",
  "stops": [
    "Улица Лизы Чайкиной",
    "Электросети",
    "Улица Докучаева",
    "Улица Лизы Чайкиной"
  ],
  "is_roundtrip": true
}
```

Описание автобусного маршрута — словарь с ключами:
`type` — строка"Bus", означающая, что объект описывает автобусный маршрут;
`name` — название маршрута;
`stops` — массив с названиями остановок, через которые проходит автобусный маршрут. У кольцевого маршрута название последней остановки дублирует название первой. Например: ["stop1", "stop2", "stop3", "stop1"];
`is_roundtrip` — значение типа `bool` Указывает, кольцевой маршрут или нет.

**Структура словаря render_settings:**

```
{
  "width": 1200.0,
  "height": 1200.0,

  "padding": 50.0,

  "line_width": 14.0,
  "stop_radius": 5.0,

  "bus_label_font_size": 20,
  "bus_label_offset": [7.0, 15.0],

  "stop_label_font_size": 20,
  "stop_label_offset": [7.0, -3.0],

  "underlayer_color": [255, 255, 255, 0.85],
  "underlayer_width": 3.0,

  "color_palette": [
    "green",
    [255, 160, 0],
    "red"
  ]
}
```

`width` и `height` — ключи, которые задают ширину и высоту в пикселях. Вещественное число в диапазоне от 0 до 100000.

`padding` — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.

`line_width` — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.

`stop_radius` — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.

`bus_label_font_size` — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.

`bus_label_offset` — смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа double. 

Задаёт значения свойств dx и dy SVG-элемента text. Элементы массива — числа в диапазоне от –100000 до 100000.

`stop_label_font_size` — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.

`stop_label_offset` — смещение названия остановки относительно её координат на карте. Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента text. Числа в диапазоне от –100000 до 100000.

`underlayer_color` — цвет подложки под названиями остановок и маршрутов.

`underlayer_width` — толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>. Вещественное число в диапазоне от 0 до 100000. color_palette — цветовая палитра. Непустой массив.

Цвет можно указать:

* в виде строки, например, `"red"` или `"black"`;
* в массиве из трёх целых чисел диапазона `[0, 255]`. Они определяют r, g и b компоненты цвета в формате `svg::Rgb`. Цвет `[255, 16, 12]` нужно вывести как `rgb(255, 16, 12)`;
* в массиве из четырёх элементов: три целых числа в диапазоне от `[0, 255]` и одно вещественное число в диапазоне от `[0.0, 1.0]`. Они задают составляющие red, green, blue и opacity цвета формата svg::Rgba. Цвет, заданный как `[255, 200, 23, 0.85]`, должен быть выведен как `rgba(255, 200, 23, 0.85)`.

**Структура словаря routing_settings**

```
"routing_settings": {
      "bus_wait_time": 6,
      "bus_velocity": 40
}
```

`bus_wait_time` — время ожидания автобуса на остановке, в минутах. Считайте, что когда бы человек ни пришёл на остановку и какой бы ни была эта остановка, он будет ждать любой автобус в точности указанное количество минут. Значение — целое число от 1 до 1000.

`bus_velocity` — скорость автобуса, в км/ч. Считайте, что скорость любого автобуса постоянна и в точности равна указанному числу. Время стоянки на остановках не учитывается, время разгона и торможения тоже. Значение — вещественное число от 1 до 1000.
Данная конфигурация задаёт время ожидания, равным 6 минутам, и скорость автобусов, равной 40 километрам в час.

### **Запросы к базе транспортного справочника**

**Запрос на получение информации об автобусном маршруте:**

```
{
  "id": 12345678,
  "type": "Bus",
  "name": "14"
}
```

Ключ name задаёт название маршрута, для которого приложение должно вывести статистическую информацию.
В ответ на этот запрос выдается в виде словаря:

```
{
  "curvature": 2.18604,
  "request_id": 12345678,
  "route_length": 9300,
  "stop_count": 4,
  "unique_stop_count": 3
}
```

В словаре содержатся ключи: curvature — число типа double, задающее извилистость маршрута. Извилистость равна отношению длины дорожного расстояния маршрута к длине географического расстояния;
`request_id` — целое число, равное id соответствующего запроса Bus;

`route_length` — целое число, равное длине маршрута в метрах;

`stop_count` — количество остановок на маршруте;

`unique_stop_count` — количество уникальных остановок на маршруте.
На кольцевом маршруте, заданном остановками A, B, C, A, количество остановок равно четырём, а количество уникальных остановок равно трём.
На некольцевом маршруте, заданном остановками A, B и C, количество остановок равно пяти (A, B, C, B, A), а уникальных — равно трём.

Запрос на получение информации об автобусной остановке:

```
{
  "id": 12345,
  "type": "Stop",
  "name": "Улица Докучаева"
}
```

Ключ name задаёт название остановки.
Ответ на запрос:

```
{
  "buses": [
      "14", "22к"
  ],
  "request_id": 12345
}
```

Запрос на получение изображения:

```
{
  "type": "Map",
  "id": 11111
}
```

Ответ на запрос:

```
{
  "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">...\n</svg>",
  "request_id": 11111
}
```

Ключ map — строка с изображением карты в формате SVG image

Запрос на построение маршрута между двумя остановками
Помимо стандартных свойств id и type, запрос содержит ещё два:

`from` — остановка, где нужно начать маршрут.

`to` — остановка, где нужно закончить маршрут.

Оба значения — названия существующих в базе остановок. Однако они, возможно, не принадлежат ни одному автобусному маршруту.

```
{
      "type": "Route",
      "from": "Biryulyovo Zapadnoye",
      "to": "Universam",
      "id": 4
}
```

Ответ на запрос:

```
{
          "items": [
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 6,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 2,
                  "time": 5.235,
                  "type": "Bus"
              },
              {
                  "stop_name": "Universam",
                  "time": 6,
                  "type": "Wait"
              },
              {
                  "bus": "635",
                  "span_count": 1,
                  "time": 6.975,
                  "type": "Bus"
              }
          ],
          "request_id": 5,
          "total_time": 24.21
      }
```

<details>
  
<summary> Пример файла make_base.json: </summary>

```
  {
      "serialization_settings": {
          "file": "transport_catalogue.db"
      },
      "routing_settings": {
          "bus_wait_time": 2,
          "bus_velocity": 30
      },
      "render_settings": {
          "width": 1200,
          "height": 500,
          "padding": 50,
          "stop_radius": 5,
          "line_width": 14,
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "stop_label_font_size": 18,
          "stop_label_offset": [
              7,
              -3
          ],
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ]
      },
      "base_requests": [
          {
              "type": "Bus",
              "name": "14",
              "stops": [
                  "Улица Лизы Чайкиной",
                  "Электросети",
                  "Ривьерский мост",
                  "Гостиница Сочи",
                  "Кубанская улица",
                  "По требованию",
                  "Улица Докучаева",
                  "Улица Лизы Чайкиной"
              ],
              "is_roundtrip": true
          },
          {
              "type": "Bus",
              "name": "24",
              "stops": [
                  "Улица Докучаева",
                  "Параллельная улица",
                  "Электросети",
                  "Санаторий Родина"
              ],
              "is_roundtrip": false
          },
          {
              "type": "Bus",
              "name": "114",
              "stops": [
                  "Морской вокзал",
                  "Ривьерский мост"
              ],
              "is_roundtrip": false
          },
          {
              "type": "Stop",
              "name": "Улица Лизы Чайкиной",
              "latitude": 43.590317,
              "longitude": 39.746833,
              "road_distances": {
                  "Электросети": 4300,
                  "Улица Докучаева": 2000
              }
          },
          {
              "type": "Stop",
              "name": "Морской вокзал",
              "latitude": 43.581969,
              "longitude": 39.719848,
              "road_distances": {
                  "Ривьерский мост": 850
              }
          },
          {
              "type": "Stop",
              "name": "Электросети",
              "latitude": 43.598701,
              "longitude": 39.730623,
              "road_distances": {
                  "Санаторий Родина": 4500,
                  "Параллельная улица": 1200,
                  "Ривьерский мост": 1900
              }
          },
          {
              "type": "Stop",
              "name": "Ривьерский мост",
              "latitude": 43.587795,
              "longitude": 39.716901,
              "road_distances": {
                  "Морской вокзал": 850,
                  "Гостиница Сочи": 1740
              }
          },
          {
              "type": "Stop",
              "name": "Гостиница Сочи",
              "latitude": 43.578079,
              "longitude": 39.728068,
              "road_distances": {
                  "Кубанская улица": 320
              }
          },
          {
              "type": "Stop",
              "name": "Кубанская улица",
              "latitude": 43.578509,
              "longitude": 39.730959,
              "road_distances": {
                  "По требованию": 370
              }
          },
          {
              "type": "Stop",
              "name": "По требованию",
              "latitude": 43.579285,
              "longitude": 39.733742,
              "road_distances": {
                  "Улица Докучаева": 600
              }
          },
          {
              "type": "Stop",
              "name": "Улица Докучаева",
              "latitude": 43.585586,
              "longitude": 39.733879,
              "road_distances": {
                  "Параллельная улица": 1100
              }
          },
          {
              "type": "Stop",
              "name": "Параллельная улица",
              "latitude": 43.590041,
              "longitude": 39.732886,
              "road_distances": {}
          },
          {
              "type": "Stop",
              "name": "Санаторий Родина",
              "latitude": 43.601202,
              "longitude": 39.715498,
              "road_distances": {}
          }
      ]
  }
  ```
</details>

<details>

<summary> Пример файла process_requests.json: </summary>

```
  {
      "serialization_settings": {
          "file": "transport_catalogue.db"
      },
      "stat_requests": [
          {
              "id": 218563507,
              "type": "Bus",
              "name": "14"
          },
          {
              "id": 508658276,
              "type": "Stop",
              "name": "Электросети"
          },
          {
              "id": 1964680131,
              "type": "Route",
              "from": "Морской вокзал",
              "to": "Параллельная улица"
          },
          {
              "id": 1359372752,
              "type": "Map"
          }
      ]
  }
```

</details>

<details>

<summary> Пример файла result.json: </summary>

```
[{
  "curvature": 1.60481,
  "request_id": 218563507,
  "route_length": 11230,
  "stop_count": 8,
  "unique_stop_count": 7
}, {
  "buses": ["14", "24"],
  "request_id": 508658276
}, {
  "items": [{
    "stop_name": "Морской вокзал",
    "time": 2,
    "type": "Wait"
  }, {
    "bus": "114",
    "span_count": 1,
    "time": 1.7,
    "type": "Bus"
  }, {
    "stop_name": "Ривьерский мост",
    "time": 2,
    "type": "Wait"
  }, {
    "bus": "14",
    "span_count": 4,
    "time": 6.06,
    "type": "Bus"
  }, {
    "stop_name": "Улица Докучаева",
    "time": 2,
    "type": "Wait"
  }, {
    "bus": "24",
    "span_count": 1,
    "time": 2.2,
    "type": "Bus"
  }],
  "request_id": 1964680131,
  "total_time": 15.96
}, {
  "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"125.25,382.708 74.2702,281.925 125.25,382.708\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"592.058,238.297 311.644,93.2643 74.2702,281.925 267.446,450 317.457,442.562 365.599,429.138 367.969,320.138 592.058,238.297\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"367.969,320.138 350.791,243.072 311.644,93.2643 50,50 311.644,93.2643 350.791,243.072 367.969,320.138\" fill=\"none\" stroke=\"red\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgb(255,160,0)\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <text fill=\"red\" x=\"50\" y=\"50\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">24</text>\n  <circle cx=\"267.446\" cy=\"450\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"317.457\" cy=\"442.562\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"125.25\" cy=\"382.708\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"350.791\" cy=\"243.072\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"365.599\" cy=\"429.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"74.2702\" cy=\"281.925\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"50\" cy=\"50\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"367.969\" cy=\"320.138\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"592.058\" cy=\"238.297\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"311.644\" cy=\"93.2643\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"black\" x=\"267.446\" y=\"450\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Гостиница Сочи</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"black\" x=\"317.457\" y=\"442.562\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Кубанская улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"125.25\" y=\"382.708\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"black\" x=\"350.791\" y=\"243.072\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Параллельная улица</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"black\" x=\"365.599\" y=\"429.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">По требованию</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"74.2702\" y=\"281.925\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"black\" x=\"50\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Санаторий Родина</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"black\" x=\"367.969\" y=\"320.138\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Докучаева</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"black\" x=\"592.058\" y=\"238.297\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n  <text fill=\"black\" x=\"311.644\" y=\"93.2643\" dx=\"7\" dy=\"-3\" font-size=\"18\" font-family=\"Verdana\">Электросети</text>\n</svg>",
  "request_id": 1359372752
}]
```
</details>

## **Системные требования**
C++17 и выше

Protobuf 3

</details>
