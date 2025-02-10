#ifndef WEBPAGE_H
#define WEBPAGE_H

const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Выбор напитков</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        table { width: 100%; max-width: 400px; margin: auto; border-collapse: collapse; }
        td { padding: 10px; font-size: 18px; text-align: center; vertical-align: middle; }
        button { font-size: 20px; width: 40px; height: 40px; border-radius: 50%; display: flex; align-items: center; justify-content: center; border: 2px solid #4CAF50; background-color: white; cursor: pointer; }
        button:hover { background-color: #f1f1f1; }
        .counter { font-size: 20px; min-width: 30px; display: inline-block; text-align: center; }
        #selectionString { font-size: 20px; margin-top: 10px; }
        
        /* Стиль для кнопки "Отправить" и "Сбросить" */
        .actionButton {
            font-size: 24px;
            width: 80%;
            max-width: 300px;
            height: 50px;
            padding: 10px 20px;
            margin-top: 15px;
            border: none;
            color: white;
            cursor: pointer;
            border-radius: 8px;
            display: block;
            margin: auto;
        }

        /* Красная кнопка сброса */
        #resetButton {
            background-color: #f44336;
        }
        #resetButton:hover {
            background-color: #e53935;
        }

        /* Зелёная кнопка отправки */
        #sendButton {
            background-color: #4CAF50;
        }
        #sendButton:hover {
            background-color: #45a049;
        }

        /* Кнопки + при достижении 4 напитков становятся тусклее */
        .inactive {
            background-color: #e0e0e0;
            cursor: not-allowed;
        }
    </style>
</head>
<body>
    <h2>Выберите напитки</h2>
    <table>
        <tr><td>1. Газированная вода</td>  <td><button onclick="changeDrink(1, -1)">-</button></td> <td><span id="count1" class="counter">0</span></td> <td><button id="plus1" onclick="changeDrink(1, 1)">+</button></td></tr>
        <tr><td>2. Мятный сироп</td>      <td><button onclick="changeDrink(2, -1)">-</button></td> <td><span id="count2" class="counter">0</span></td> <td><button id="plus2" onclick="changeDrink(2, 1)">+</button></td></tr>
        <tr><td>3. Апельсиновый сок</td>  <td><button onclick="changeDrink(3, -1)">-</button></td> <td><span id="count3" class="counter">0</span></td> <td><button id="plus3" onclick="changeDrink(3, 1)">+</button></td></tr>
        <tr><td>4. Лимонад “Мятный”</td>  <td><button onclick="changeDrink(4, -1)">-</button></td> <td><span id="count4" class="counter">0</span></td> <td><button id="plus4" onclick="changeDrink(4, 1)">+</button></td></tr>
        <tr><td>5. Лимонад “Заводной апельсин”</td> <td><button onclick="changeDrink(5, -1)">-</button></td> <td><span id="count5" class="counter">0</span></td> <td><button id="plus5" onclick="changeDrink(5, 1)">+</button></td></tr>
        <tr><td>6. Лимонад “Тройной”</td> <td><button onclick="changeDrink(6, -1)">-</button></td> <td><span id="count6" class="counter">0</span></td> <td><button id="plus6" onclick="changeDrink(6, 1)">+</button></td></tr>
    </table>

    <p id="selectionString"></p> <!-- Здесь будет отображаться выбранные напитки -->

    <!-- Кнопка сброса -->
    <button id="resetButton" class="actionButton" onclick="resetSelection()">Сбросить</button> 

    <!-- Кнопка отправки -->
    <button id="sendButton" class="actionButton" onclick="sendSelection()">Отправить</button>

    <script>
        let selectionOrder = [];
        let selectionCount = {};

        function changeDrink(num, delta) {
            if (!selectionCount[num]) selectionCount[num] = 0;

            if (delta > 0 && selectionOrder.length < 4) {
                selectionOrder.push(num);
                selectionCount[num]++;
            } else if (delta < 0 && selectionCount[num] > 0) {
                let index = selectionOrder.lastIndexOf(num);
                if (index !== -1) selectionOrder.splice(index, 1);
                selectionCount[num]--;
            }

            updateSelection();
        }

        function updateSelection() {
            let selectionStr = selectionOrder.join("") || ""; // Пустая строка, если нет выбора
            document.getElementById("selectionString").textContent = "Выбранные: " + selectionStr;

            for (let i = 1; i <= 6; i++) {
                document.getElementById("count" + i).textContent = selectionCount[i] || 0;
                // Управление состоянием кнопок +
                if (selectionOrder.length >= 4) {
                    document.getElementById("plus" + i).classList.add("inactive");
                } else {
                    document.getElementById("plus" + i).classList.remove("inactive");
                }
            }
        }

        function sendSelection() {
            let selectionStr = selectionOrder.join("");
            if (selectionStr) fetch(`/command?cmd=${selectionStr}`);
        }

        function resetSelection() {
            selectionOrder = [];
            selectionCount = {};
            updateSelection();
        }
    </script>
</body>
</html>
)rawliteral";

#endif
