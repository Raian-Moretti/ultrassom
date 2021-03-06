from threading import Thread
import requests
import os
import asyncio
import websockets
import json

class API():

    def __init__(self, ) -> None:
        self.BASE_URL = os.getenv('BASE_URL') or 'http://127.0.0.1:8000'
        self.WS = None

        self.start_processing = None
        self.finish_processing = None
        self.failed_processing = None
        self.enqueued = None

    # Send a POST request to /images
    def send_image(self, body: dict, image: str) -> int:
        ## Send the creation request
        response_create = requests.post(f'{self.BASE_URL}/images', json=body)

        if response_create.status_code != 201:
            print('Error while creating image:', response_create.content)
            return -1
        image_id = response_create.json()['id']

        ## Upload image content as plain text
        response_signal = requests.post(f'{self.BASE_URL}/images/{image_id}/signal', data=image, headers={'Content-Type': 'text/csv'})
        if response_signal.status_code != 202:
            print('Error while sending image:', response_signal.content)
            return -1

        return image_id

    # Send a GET request to /users/:name/images
    def get_images(self, user_id: int, user_name: str) -> dict:
        response = requests.get(f'{self.BASE_URL}/users/{user_id}/images').json()

        for i in range(len(response)):
            if response[i]["data"] != None:
                response[i]['image_url'] = f'{self.BASE_URL}/images/{response[i]["data"]}'

        return response

    # Send a GET request to /images/:id to download an image
    def download_image(self, image_url: str): 
        response = requests.get(image_url)

        if not response.ok:
            return -1

        return response.content

    # Send a POST request to /users to create a new user
    def create_user(self, name: str) -> int:
        response = requests.post(f'{self.BASE_URL}/users', json={'name': name})

        if response.status_code != 201:
            return -1

        return response.json()['id']

    # Send a GET request to /users with query parameter name
    def get_user(self, name: str) -> int:
        response = requests.get(f'{self.BASE_URL}/users', params={'name': name})

        if response.status_code != 200:
            return -1

        return response.json()['id']

    # Create websocket connection with /users/:id/ws
    def open_websocket(self, user_id: int, callbacks: dict):
        self.start_processing = callbacks['on_start_processing']
        self.finish_processing = callbacks['on_finish_processing']
        self.failed_processing = callbacks['on_failed_processing']
        self.enqueued = callbacks['on_enqueued']

        def run_thread(self):
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)

            async def run(BASE_URL, on_message):
                WS_BASE_URL = BASE_URL.replace('http', 'ws')
                WS_URL = f'{WS_BASE_URL}/users/{user_id}/ws'
                try:
                    WS = await websockets.connect(WS_URL, ping_timeout=None)
                    while True:
                        message = await WS.recv()
                        on_message(message)
                except Exception as e:
                    print('Error while connecting to websocket:', e)
                    return

            asyncio.get_event_loop().run_until_complete(run(self.BASE_URL, self.on_message))

        self.WST = Thread(target=run_thread, args=(self,))
        self.WST.daemon = True
        self.WST.start()

    def on_message(self, message):
        content = json.loads(message)
        evt = content['type']
        data = content['image']

        if data["data"] != None:
            data['image_url'] = f'{self.BASE_URL}/images/{data["data"]}'

        def handle_start_processing(self, data):
            if self.start_processing != None:
                self.start_processing(data)

        def handle_finish_processing(self, data):
            if self.finish_processing != None:
                self.finish_processing(data)

        def handle_failed_processing(self, data):
            if self.failed_processing != None:
                self.failed_processing(data)

        def handle_enqueued(self, data):
            if self.enqueued != None:
                self.enqueued(data)

        switch = {
            'ENQUEUED': handle_enqueued,
            'START_PROCESSING': handle_start_processing, 
            'FINISH_PROCESSING': handle_finish_processing,
            'FAILED_PROCESSING': handle_failed_processing,
        }
        case = switch.get(evt)

        if (case != None):
            case(self, data)