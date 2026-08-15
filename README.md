RU:

BinaryScript (BS)

BinaryScript — это язык программирования, основанный на идее полного отказа от букв в пользу чисел и бинарного кода. Язык максимально приближен к машинному представлению данных, но при этом остаётся читаемым и удобным для человека.

Версия движка: 0.30
Поддержка: Windows, Linux, macOS

---

📌 Основная концепция

В BinaryScript всё — числа. Переменные, условия, циклы, ввод/вывод — всё построено на цифровых операциях и бинарных строках. Язык не использует привычные ключевые слова (if, print, set), заменяя их на числовые команды.

Это не просто синтаксический сахар — это попытка возродить идею числового программирования, сделав его доступным и практичным.

---

🧠 Команды языка

```
Команда Назначение Пример
0 Присваивание переменной 0 x = 1
0 ! Запрос ввода от пользователя 0 !name =
2() Вывод числа 2(10 + 20)
2([]) Вывод текста 2([Hello World])
3 Условный оператор if 3 x == 1: 2(x)
34 else if 34 x == 0: 2(x)
4 else 4: 2(0)
6 Цикл с условием или счётчиком 6 (x < 5): 2(x);
8 Бесконечный цикл 8: 2([loop]);
1[] Работа с файлами и системой 1[sys], 1[file.bs](run)
```

---

🔧 Системные возможности

```
Команда Описание
1[time] Вывод текущего времени (HH:MM:SS)
1[sys] Вывод информации о системе (ОС, ядра, версия движка)
1[путь](read) Чтение содержимого файла
1[путь](write) Создание и запись файла
1[путь](edit) Редактирование существующего файла
1[https://...] Открытие ссылки в браузере
```

---

🧪 Пример кода

```binaryscript
0 x = 10
3 x == 10: 2([Значение X равно 10]);
4: 2([X не равен 10]);
```

---

🧬 Особенности синтаксиса

```
· ; — завершает блок (условие, цикл)
· & — разделитель команд в одной строке
· # ... # — многострочные комментарии
· {} — строковые литералы в выражениях
· Бинарные строки: {01001000_01100101} → He
```

---

🖥️ Поддерживаемые платформы

· ✅ Linux
· ✅ Windows
· ✅ macOS

---

🚀 Участие в разработке

Проект находится на стадии активного бета-тестирования.
Если вы хотите поучаствовать в разработке — пишите в ТГ @DmOS01

---

🔄 Обновления

```
до v0.30:
•   на канале @nyan_pl
====================
ожидается...
```

---

📜 Лицензия

Проект распространяется свободно. Поддержите разработку, чтобы мы могли сделать BinaryScript ещё удобнее и мощнее!

PayPal: @DimaOS01

Monobank/Visa: 4441114439762566

---

«BinaryScript — язык, который говорит с машиной на её языке.»

===

EN:

BinaryScript (BS)

BinaryScript is an programming language based on the idea of completely eliminating letters in favor of numbers and binary code. The language is as close as possible to machine data representation, while remaining readable and user-friendly.

Engine version: 0.30
Support: Windows, Linux, macOS

---

📌 Basic Concept

In BinaryScript, everything is numbers. Variables, conditions, loops, input/output—everything is built on digital operations and binary strings. The language does not use traditional keywords (if, print, set), replacing them with numeric commands.

This is not just syntactic sugar—it is an attempt to revive the idea of numerical programming, making it accessible and practical.

---

🧠 Language Commands

```
Command Purpose Example
0 Assigning variable 0 x = 1
0 ! Request user input 0 !name =
2() Output a number 2(10 + 20)
2([]) Output text 2([Hello World])
3 Conditional statement if 3 x == 1: 2(x)
34 else if 34 x == 0: 2(x)
4 else 4: 2(0)
6 Loop with a condition or counter 6 (x < 5): 2(x);
8 Infinite loop 8: 2([loop]);
1[] Working with files and the system 1[sys], 1[file.bs](run)
```

---

🔧 System Features

```
Command Description
1[time] Displays the current time (HH:MM:SS)
1[sys] Displays system information (OS, kernels, engine version)
1[path](read) Reads file contents
1[path](write) Creates and writes a file
1[path](edit) Edits an existing file
1[https://...] Opens a link in a browser
```

---

🧪 Code Example

```binaryscript
0 x = 10
3 x == 10: 2([The value of X is 10]);
4: 2([X is not equal to 10]);
```

---

🧬 Syntax Features

```
· ; — terminates a block (condition, loop)
· & — command separator on a single line
· # ... # — multi-line comments
· {} — string literals in expressions
· Binary strings: {01001000_01100101} → He
```

---

🖥️ Supported Platforms

· ✅ Linux
· ✅ Windows
· ✅ macOS

---

🚀 Participate in Development

The project is in active beta testing.
If you would like to participate in the development, please contact @DmOS01 in Telegram.

---

🔄 Updates

```
up to v0.30:
• on the @nyan_pl channel
======================
expected...
```

---

📜 License

The project is freely distributed. Support the development so we can make BinaryScript even more convenient and powerful!

PayPal: @DimaOS01

Monobank/Visa: 4441114439762566

---

"BinaryScript is a language that speaks to the machine in its own language."
