import datetime 
import json
import requests
import matplotlib.pyplot as plt
import numpy as np
import matplotlib as mpl


def get_request(url):
    req = requests.get(url)

    if req.status_code != 200:
        raise Exception('Erro de autenticação')

    return req.content


url = "https://iiot-dta-default-rtdb.firebaseio.com/challenge02.json"

# proxy = {
#     "http":"http://disrct:ets%40bosch207@rb-proxy-ca1.bosch.com:8080",
#     "https":"http://disrct:ets%40bosch207@rb-proxy-ca1.bosch.com:8080"
# }

# auth = requests.auth.HTTPProxyAuth("disrct", "ets@bosch207")

def update_data():
    dados = json.loads(get_request(url)) #carrega dados em json
    dados_len = len(dados)
    indices = np.array([int(x[-2:]) for x in dados.keys()])

    humidity = np.full(dados_len, np.nan, dtype=np.float64)
    temperature = np.full(dados_len, np.nan, dtype=np.float64)

    for i, j in zip(range(dados_len),indices): #não poderia ser dois FOR
        try:
            humidity[i] = dados[f"subsys_{j:02}"]["humidity"]
            temperature[i] = dados[f"subsys_{j:02}"]["temperature"]
        except KeyError:
            pass

    humidity_mean = np.mean(humidity[~np.isnan(humidity)])
    temperature_mean = np.mean(temperature[~np.isnan(temperature)])

    return humidity_mean, temperature_mean

fig, axs = plt.subplots(2, sharex=True, figsize=(16, 8), gridspec_kw={"hspace":0.4}) #retorna o grafico e os eixos
fig.supxlabel("Tempo")
ax_humidity, ax_temperature = axs

cmap = mpl.colormaps["coolwarm"]

ax_humidity.grid(True)
ax_humidity.set_ylabel("Umidade")

ax_temperature.grid(True)
ax_temperature.set_ylabel("Temperatura")


plt.ion()
fig.show()
fig.canvas.draw()

while True:
    humidity_mean, temperature_mean = update_data()
    current_time = datetime.datetime.now()

    #(x - media) / (maximo - minimo)
    temp_color = (temperature_mean - 30)/(35-25)

    ax_humidity.plot(current_time, humidity_mean, linestyle='', marker='o', markersize=5, color='r')
    ax_temperature.plot(current_time, temperature_mean, linestyle='', marker='o', markersize=5, color=cmap(temp_color))

    fig.canvas.draw()
    plt.pause(3)

# #fazendo grafico
# plt.plot(indices, humidity, linestyle='', maker='o', markersize=5, color='r')
# plt.grid()
# plt.xlabel("Indices")
# plt.ylabel("Luminosidade")
# plt.show()

# plt.plot(indices, temperature, linestyle='', maker='o', markersize=5, color='r')
# plt.grid()
# plt.xlabel("Indices")
# plt.ylabel("Temp sensor 0")
# plt.show()

# plt.plot(indices, temp_sensor_01, linestyle='', maker='o', markersize=5, color='r')
# plt.grid()
# plt.xlabel("Indices")
# plt.ylabel("Temp sensor 1")
# plt.show()
