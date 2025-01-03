import matplotlib.pyplot as plt

# Считывание данных из файла
y1_values = []
y2_values = []

with open("results.txt", "r") as file:
    for line in file:
        y1, y2 = line.split()
        y1_values.append(float(y1))
        y2_values.append(float(y2))

# Построение графика зависимости y1 от y2
plt.figure(figsize=(8, 6))
plt.plot(y1_values, y2_values, label='y1 vs y2', color='b')

plt.xlabel('y1')
plt.ylabel('y2')
plt.title('Van der Pol Oscillator - y1 vs y2')
plt.grid(True)
plt.legend()

# Показать график
plt.tight_layout()
plt.show()
