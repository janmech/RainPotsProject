import time
import json
import traceback
import asyncio


class Listener:
    def __init__(self, port_listen_to, params, serial_sender, debug: bool):
        self.params = params
        self.grab_param_values = False
        self.debug = debug
        self.serial_sender = serial_sender

    def meter_message(self, path, *args):
        meter_index, meter_value = args
        meter_index = int(meter_index)
        meter_value = int(meter_value * 255.)
        if self.debug:
            print("Meter: %d %d" % (meter_index, meter_value))
        self.serial_sender.send_meter(meter_index, meter_value)

    def response(self, path, *args):
        print("\t RESPONSE ENDPOINT:", args)
        try:  # Try to reload config after new export to target
            response = json.loads(args[0])
            message = response['result']['message']
            progress = response['result']['progress']
            if self.debug:
                print("\tMessage:", message)
                print("\tProgress:", progress)
            if message == 'loaded' and progress == 100:
                self.params.load_config()
        except Exception as err:
            if self.debug:
                print(str(err))
                traceback.print_exc()
            pass

    def fallback(self, path, *args):
        # print(f"{path}: {args}")
        # Normalized param values are sent out first, so we uae it as start flag
        # if path == '/rnbo/inst/0/params/reload-config-start/normalized':
        if path == '/rnbo/inst/0/presets/load':
            if self.debug:
                print(f"PRESET {args[0]} RELOAD ---START---")
                print("Resetting Button Values and starting collecting")
            self.params.reset_values_by_path()
            self.params.set_grab_values(True)

        # None Normalized param values are sent out second, so we uae it as end flag
        if path == '/rnbo/inst/0/params/reload-config-end':
            if self.debug:
                print("PRESET RELOAD ---FINISHED---")
                print(self.params.get_values_by_path())
            self.params.set_grab_values(False)
            time.sleep(0.1)
            self.serial_sender.send_button_values()

        if self.params.get_grab_values():
            if self.debug:
                print("fallback", path, args)
            if path.endswith('/normalized') and path in self.params.get_values_by_path().keys():
                truncated = int(args[0] * 1000) / 1000
                self.params.get_values_by_path()[path] = truncated
                if self.debug:
                    print("   -->Setting Value:   ", path, self.params.get_values_by_path()[path])
        elif self.debug:
            print("\t OSC Message:", path, args)
