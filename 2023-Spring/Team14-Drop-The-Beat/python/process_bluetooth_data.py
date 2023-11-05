import asyncio
import bleak
import utils.spotify_calls as spotify

UP = 0
LEFT = 1
DOWN = 2
RIGHT = 3

########!!!!!!
# Following address  variable needs to be changes based on Arduino used!
########!!!!!!
address = "35:40:8a:32:68:c0" # Address of Arduino Periperal that is sending Bluetooth data
characteristic_uuid = "33f5a498-e565-11ed-b5ea-0242ac120002"

async def run():
    while True:
    # Connect to the Arduino 
        async with bleak.BleakClient(address, timeout=30.0) as client:
            # Read the value of the characteristic
            value = await client.read_gatt_char(characteristic_uuid)
            int_val = int.from_bytes(value, "big")

            # TODO on somme specific value do stuff
            print(f"Characteristic value: {int_val}")
            if int_val == UP:
                spotify.play_pause()
                print("\nUP gesture")
                print("\nTrigger play/pause...\n")
            elif int_val == LEFT:
                spotify.previous_track()
                spotify.previous_track()
                print("\nLEFT gesture")
                print("\nPlay previous track...\n")
            elif int_val == DOWN:
                spotify.previous_track()
                print("\nDOWN gesture")
                print("\nGo back to start of track...\n")
            elif int_val == RIGHT:
                spotify.next_track()
                print("\nRIGHT gesture")
                print("\nPlay next track...")
            else:
                print("Detected motion is not associated with any instruction.\n")

asyncio.run(run())