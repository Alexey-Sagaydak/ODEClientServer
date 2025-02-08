# Выпускная квалификационная работа бакалавра

*Тема: Автоматизация построения алгоритмов интегрирования на основе явных численных схем с согласованными областями устойчивости.*

## Описание

В рамках данной работы требуется разработать адаптивный алгоритм интегрирования, содержащий несколько стадий, контролирующий точность и устойчивость на каждом шаге. Это позволит решать системы дифференциальных уравнений, включая жесткие системы.

**Проект состоит из двух частей:**

1. **Сервер** — получает задачу, решает её и возвращает массив точек клиенту.

2. **Клиент** — это приложение, созданное для тестирования различных решателей.

**Структура проекта:**

```
ODEClientServer/
├── Client/ # Клиентская часть приложения
│   ├── Images/ # Изображения
│   ├── Models/ # Модели данных
│   ├── ViewModels/ # Логика представления
│   └── Views/ # Пользовательский интерфейс
│
├── Server/ # Серверная часть приложения
│   ├── contrib/ # Внешние зависимости
│   ├── src/ # Исходный код сервера
│   └── odesolvers-lib/ # Библиотека с решателями ОДУ
│
└── Docker/ # Конфигурация Docker для сборки сервера
```

**Основные возможности:**

- **Сервер**:
  - Принимает запросы от клиента;
  - Решает задачу заданным решателем;
  - Возвращает результаты вычислений клиенту.

- **Клиент**:
  - Выбирает готовую систему ОДУ из списка;
  - При необходимости меняет коэффициенты задачи;
  - Выбирает решатель;
  - Отправляет запрос серверу на решение задачи;
  - Получает решение и строит график.

## Сборка и запуск

### Сервер

1. Убедитесь, что в системе установлен Docker;
2. Для запуска сервера достаточно ввести команду:
    ```bash
    docker run -it -d -p 7777:7777 --rm alexey123789/ode-server:latest
    ```
3. Чтобы убедиться, что сервер работает, введите в браузере следующий адрес:
    ```
    http://127.0.0.1:7777
    ```
    В случае успеха появится сообщение `Server is working fine!`

> [!NOTE]
> Если необходимо выполнить сборку сервера из исходного кода, то обратитесь к инструкции в папке [Docker/](Docker/).

### Клиент

1. Требования:
   - Windows 7, 10/11;
   - .NET 8.0 и выше.
2. Для запуска клиента запустите `.exe` файл из раздела `Releases` репозитория.

> [!NOTE]
> Если необходимо выполнить сборку клиента из исходного кода, то рекомендуется открыть проект в `Visual Studio 2022` или `JetBrains Rider` с установленными компонентами для работы с `WPF` на `C#`.

## Описание интерфейса клиента

![Описание интерфейса клиента](https://github.com/user-attachments/assets/1392fc30-0671-4b18-bfb0-b255b71406ff)

### Меню
   | Пункт меню | Подпункты |
   | ------ | ------ |
   | Файл | ![Файл](https://github.com/user-attachments/assets/8920ed8b-1e62-4e08-be6c-702f88418dbb) |
   | Настройки | ![Настройки](https://github.com/user-attachments/assets/53cfe64a-b236-4dbb-824d-ce24a166bf80) |
   | Текст | ![Текст](https://github.com/user-attachments/assets/c27dee81-527b-439c-a866-01b7fcfbf677) |
   | Справка | ![Справка](https://github.com/user-attachments/assets/093a9dff-3dcc-45f7-aaa8-04a2f7722bf8) |
   
### Панель инструментов
   
   ![Панель инструментов](https://github.com/user-attachments/assets/902588fd-fd8b-427e-9279-151009d6684d)

   - Сохранить результаты в виде текста (`.xlsx`, `.txt`)
   - Сохранить графики как изображение (`.jpg`)
   - Ось X
   - Ось Y
   - Увеличить масштаб
   - Уменьшить масштаб
   - Сбросить масштаб
   - Очистить поле с графиками
   - `Xmin-Xmax` - диапазон по оси X
   - `Ymin-Ymax` - диапазон по оси Y
   - Статус подключения к серверу
   - Вызов справки
   - О программе

### Заполнение информации о задаче

  Для тестирования решателей было добавлено три задачи:

  | Задача | Параметры |
  | ------ | ------ |
  | Уравнение Ван дер Поля | ![Уравнение Ван дер Поля](https://github.com/user-attachments/assets/8f2530af-fbbf-4395-9253-c6e7ac6001c4) |
  | Принужденный осциллятор | ![Принужденный осциллятор](https://github.com/user-attachments/assets/3ad009ae-e36f-41b6-9471-8feeed11267f) |
  | Система Робертсона | ![Система Робертсона](https://github.com/user-attachments/assets/e7cd850c-b4aa-455d-8d67-a1389edb3e9f) |

### Выбор метода решения

![Методы решения](https://github.com/user-attachments/assets/bc777274-6a5a-40f6-a1b8-38471227d640)

#### 1. Алгоритмы с контролем точности
- [x] **Явный метод Эйлера** — простейший метод на основе формулы явного метода Эйлера для прогноза, а трапеций — для коррекции.

- [x] **RK21** — основан на двухстадийной формуле типа Рунге-Кутта с контролем точности.

#### 2. Алгоритмы с контролем устойчивости
- [x] **RK23S** — трехстадийный метод типа Рунге-Кутты с контролем точности и устойчивости.

- [x] **STEKS** — пятистадийный метод четвертого порядка точности типа Рунге-Кутта с контролем точности и устойчивости.

#### 3. Алгоритмы переменного порядка и шага
- [x] **DISPD** — метод, состоящий из двух схем, с переменным порядком и шагом, адаптирующийся к изменению поведения решения.

- [x] **DISPF** — алгоритм переменного порядка и шага на основе численных формул первого, второго и пятого порядков точности.
   | Пункт меню | Подпункты |
   | --- | ------ |
   | Описание параметров метода DISPF</br>(появляется при нажатии на знак подсказки) | ![Описание параметров DISPF](https://github.com/user-attachments/assets/0d459cdd-d86a-401e-8164-aebbf26a734e) |

> [!NOTE]
> Более подробно можно прочитать в документации к библиотеке решателей в [Server/src/odesolvers-lib/doc/](Server/src/odesolvers-lib/doc/).

## Описание API сервера

### 1. Проверка работоспособности сервера
- **Метод:** `GET`
- **Путь:** `/`
- **Ответ:** `Server is working fine!`

### 2. Решение системы ОДУ (асинхронный потоковый режим)
- **Метод:** `POST`
- **Путь:** `/solve`
- **Тело запроса (`JSON`):**  
  ```json
  {
    "Method": "Метод решения",
    "Equation": "Имя задачи",
    "Parameters": {
      "t0": 0.0,
      "t1": 10.0,
      "tolerance": 1e-5,
      "y0": [1.0, 0.0]
    }
  }
  ```
- **Ответ:**
  - **Формат:** потоковый `JSON` с использованием `Transfer-Encoding: chunked`
  - **Структура:**  
    ```json
    {
      "status": "success | error",
      "results": [
        {"t": 0.0, "values": [1.0, 0.0]},
        {"t": 0.1, "values": [0.99, -0.1]}
      ]
    }
    ```
  - **Особенности:**
    - Данные отправляются чанками по мере вычисления
    - При ошибке возвращается `HTTP 500` с описанием

## Лицензия

Данный проект распространяется под лицензией MIT. Подробнее см. файл [LICENSE](LICENSE).
