import requests

token = "c2e736fa77bafb4b024e5e72d19f35ea10c1fb7fc73c4eeede4b064c3e8503a7"

headers = {
    "Authorization": "Bearer %s" % token,
}

# get info about light 
# response = requests.get('https://api.lifx.com/v1/lights/id:d073d53ed098', headers=headers)

# toggle light
# response = requests.post('https://api.lifx.com/v1/lights/id:d073d53ed098/toggle', headers=headers)

# set light status 
payload = {
    "power": "on",
}

response = requests.put('https://api.lifx.com/v1/lights/id:d073d53ed098/state', data=payload, headers=headers)

# print(response.headers)

# print(response.text)