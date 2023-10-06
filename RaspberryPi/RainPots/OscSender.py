from pythonosc.udp_client import SimpleUDPClient
import RainPots.Parameters
import RainPots.SerialSender


class Sender:

    def __new__(cls, port_in: int, port_out: int, address_destination: str, params: RainPots.Parameters,
                serial_sender: RainPots.SerialSender.Sender,
                debug: bool):
        obj = object.__new__(cls)
        return obj

    def __init__(self, port_listen_to: int, port_send_to: int, address_destination: str, params: RainPots.Parameters,
                 serial_sender: RainPots.SerialSender.Sender,
                 debug: bool) -> None:
        self.port_send_to = port_send_to
        self.params = params
        self.serial_sender = serial_sender
        self.debug = debug
        self.osc_client = SimpleUDPClient(address_destination, port_send_to)

    def add_listener(self, port) -> None:
        if self.debug:
            print('Adding Listener: 127.0.0.1:%d' % port)
        self.osc_client.send_message('/rnbo/listeners/add', '127.0.0.1:%d' % port)

    def send_pgm_control(self, cmd: str, pgm_index: int) -> None:
        preset_name = str(pgm_index).zfill(3)
        if cmd == 'save' and pgm_index > 0:  # Preset index 0 is our init preset: Do not overwrite
            path = '/rnbo/inst/0/presets/save'
        elif cmd == 'load':
            path = '/rnbo/inst/0/presets/load'
        else:
            return
        if self.debug or True:
            print("Sending: ", path, preset_name)
        self.osc_client.send_message(path, preset_name)

    def send_packet(self, packet) -> None:
        rainpots_unit = packet[0] & 0x0f
        controller = packet[1]
        value = (packet[3] << 7) | packet[2]
        normalized_value = self.params.get_normalized_value(rainpots_unit, controller, value)
        if normalized_value != -1:
            path = self.params.get_config()[rainpots_unit][controller]['path']
            try:
                current_param_value = self.params.get_values_by_path()[path]
            except KeyError:
                current_param_value = None

            # Parameter pickup
            # We don't have a value or if it's a button: send the incoming one
            if current_param_value is None or self.params.is_button(controller):
                self.osc_client.send_message(path, normalized_value)
                self.params.set_controller_state(rainpots_unit, self.params.PICKUP_VALUE_LOCKED)
                if self.debug:
                    print("Sending: ", path, normalized_value)
            elif abs(
                    current_param_value - normalized_value) <= 0.03:  # We do have a value: Check if it can be picked up
                self.params.get_values_by_path()[path] = None  # Param is picked up after preset load: remove the value
                self.osc_client.send_message(path, normalized_value)
                self.params.set_controller_state(rainpots_unit, self.params.PICKUP_VALUE_LOCKED)
                if self.debug:
                    print("Sending: ", path, normalized_value)
            else:
                controller_state = self.params.PICKUP_VALUE_UP
                if current_param_value < normalized_value:
                    controller_state = self.params.PICKUP_VALUE_DOWN
                self.params.set_controller_state(rainpots_unit, controller_state)
                if self.debug:
                    print("Needs pickup: %s %f %f" % (path, current_param_value, normalized_value))
            # self.serial_sender.send_pickup(rainpots_unit)

        else:
            if self.debug:
                print("Unit %d Controller %d NOT Configured [%d]" % (rainpots_unit, controller, value))

    def get_param(self) -> None:
        self.osc_client.send_message('rnbo/inst/0/params')
