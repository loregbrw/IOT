import datetime 
import json
import requests
import matplotlib.pyplot as plt

def get_request(url, proxy, auth):
    req = requests.get(url, proxies=proxy, auth=auth)

    if req.status_code != 200:
        raise Exception("Erro de autenticação")

    return req.content

url = "https://iiot-dta-default-rtdb.firebaseio.com/avaliacao/subsys_11.json"

proxy = {
    "http": "http://disrct:ets%40bosch207@rb-proxy-ca1.bosch.com:8080",
    "https": "http://disrct:ets%40bosch207@rb-proxy-ca1.bosch.com:8080"
}

auth = requests.auth.HTTPProxyAuth("disrct", "ets@bosch207")

def update_data():
    dados = json.loads(get_request(url))
    mean_temp = dados["temperatura_media"]
    dev_temp = dados["desvio_padrao"]
    
    return mean_temp, dev_temp
    

fig, axs = plt.subplots(2, sharex=True, figsize=(16, 8), gridspec_kw={"hspace":0.4}) 
fig.supxlabel("Tempo")
ax_mean, ax_dev = axs

ax_mean.grid(True)
ax_mean.set_ylabel("Média")

ax_dev.grid(True)
ax_dev.set_ylabel("Desvio padrão")

plt.ion()
fig.show()
fig.canvas.draw()

while True:
    temperature_mean, temperature_dev = update_data()
    current_time = datetime.datetime.now()

    ax_mean.plot(current_time, temperature_mean, linestyle='', marker='o', markersize=5, color='r')
    ax_dev.plot(current_time, temperature_dev, linestyle='', marker='o', markersize=5, color='b')

    fig.canvas.draw()
    plt.pause(1)