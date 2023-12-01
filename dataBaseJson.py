import datetime, requests, json
import matplotlib.pyplot as plt
import numpy as np

# bem2ct
# gaussKronrod754


def get_request(url, proxy, auth):
    req = requests.get(url, proxies=proxy, auth=auth)

    if req.status_code != 200:
        raise Exception("Erro na requisicao")

    return req.content


url = "https://iiot-dta-default-rtdb.firebaseio.com/iiot-dta.json"

proxy = {
    "http": "http://disrct:ets%40bosch207@rb-proxy-ca1.bosch.com:8080",
    "https": "http://disrct:ets%40bosch207@rb-proxy-ca1.bosch.com:8080"
}

auth = requests.auth.HTTPProxyAuth("disrct", "ets@bosch207")


def update_data():
    data = json.loads(get_request(url, proxy, auth))
    data_len = len(data)
    indices = np.array([int(x[-2:]) for x in data.keys()])

    luminosity = np.full(data_len, np.nan, dtype=np.float64)
    temp_sensor_00 = np.full(data_len, np.nan, dtype=np.float64)
    temp_sensor_01 = np.full(data_len, np.nan, dtype=np.float64)
    for i, j in zip(range(data_len), indices):
        try:
            luminosity[i] = data[f"subsys_{j:02}"]["luminosity"]
            temp_sensor_00[i] = data[f"subsys_{j:02}"]["temperature"]["temp_sensor_00"]
            temp_sensor_01[i] = data[f"subsys_{j:02}"]["temperature"]["temp_sensor_01"]
        except KeyError:
            pass

    luminosity_mean = np.mean(luminosity[~np.isnan(luminosity)])
    temp_sensor_00_mean = np.mean(temp_sensor_00[~np.isnan(temp_sensor_00)])
    temp_sensor_01_mean = np.mean(temp_sensor_01[~np.isnan(temp_sensor_01)])
    
    return luminosity_mean, temp_sensor_00_mean, temp_sensor_01_mean

fig, axs = plt.subplots(3, sharex=True, figsize=(16, 8), gridspec_kw={"hspace": 0.4})
fig.supxlabel("Tempo")
ax_luminosity, ax_temp_sensor_00, ax_temp_sensor_01 = axs

ax_luminosity.grid()
ax_luminosity.set_ylabel("Luminosidade")

ax_temp_sensor_00.grid()
ax_temp_sensor_00.set_ylabel("Temperatura Sensor 0")

ax_temp_sensor_01.grid()
ax_temp_sensor_01.set_ylabel("Temperatura Sensor 1")

plt.ion()
fig.show()
fig.canvas.draw()

while True:
    tempo_atual = datetime.datetime.now()
    luminosity_mean, temp_sensor_00_mean, temp_sensor_01_mean = update_data()
    

    color_temp = 'b' if temp_sensor_00_mean < 29.5 else 'r'
    
    ax_luminosity.plot(tempo_atual, luminosity_mean, linestyle='', marker='o', markersize=5, color=color_temp)
    ax_temp_sensor_00.plot(tempo_atual, temp_sensor_00_mean, linestyle='', marker='o', markersize=5, color=color_temp)
    ax_temp_sensor_01.plot(tempo_atual, temp_sensor_01_mean, linestyle='', marker='o', markersize=5, color=color_temp)
 
    fig.canvas.draw()

    plt.pause(1)