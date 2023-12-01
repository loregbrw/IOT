import json
import requests

url = "https://random.dog/woof.json"

proxy = {
    "http": "http://rb-proxy-ca1.bosch.com:8080",
    "https": "http://rb-proxy-ca1.bosch.com:8080"
}

auth = requests.auth.HTTPProxyAuth("disrct", "ets@bosch207")

req = requests.get(url)

if req.status_code != 200:
    raise Exception("Erro de requisicao")

data = json.loads(req.content)

url_image = data["url"]

with open("cachorro.jpg", "wb") as image:
    req_image = requests.get(url_image)
    if req_image.status_code != 200:
        raise Exception("Erro de requisicao")

    image.write(req_image.content)