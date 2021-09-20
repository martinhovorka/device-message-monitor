#!/usr/bin/python3
"""
Main module for device simulator for backend testing

Script will attempt to generate random data for random number of devices for
given amount of time.

1) generate random number of devices with random names (default 1-100 devices)
    ... or ...
   generate specific set of named devices
   1.1) each device can measure from 0 up to 3 values. What values  
        the device measures is chosen randomly. 
2) during execution
    2.1) randomly decide if device should generate new message with measurements
    2.2) if decided, randomly decide to generate temperature and/or voltage
         and/or current measurements
    NOTE: message may contain 0 - 3 measured values
"""

################################################################################

import http.client
import json
import time
import random
import string

################################################################################


def random_boolean():
    """
    function generates random bool value
    """
    return random.randint(0, 1) == 0


class Measurements:
    """
    class holds all totals of measurements
    """

    def __init__(self, device_name, temperature_enabled, voltage_enabled, current_enabled):
        self.__device_name = device_name
        self.__temp_enabled = temperature_enabled
        self.__volt_enabled = voltage_enabled
        self.__cur_enabled = current_enabled
        self.total_count = 0
        self.temperature_count = 0
        self.overheat_faults = 0
        self.voltage_count = 0
        self.overvoltage_faults = 0
        self.undervoltage_faults = 0
        self.current_count = 0
        self.overcurrent_faults = 0
        self.no_measurements = 0

    def __str__(self):
        outstr = """
--------------------------------------------------------------------------------
device name              = {0} (current: {1}; voltage: {2}; temperature: {3})
total messages           = {4}
current measurements     = {5} (overcurrent faults {6})
voltage measurements     = {7} (undervoltage faults {8}, overvoltage faults {9})
temperature measurements = {10} (overheat faults {11})
no measurements          = {12}
--------------------------------------------------------------------------------
""".format(self.__device_name,
           self.__cur_enabled, self.__volt_enabled, self.__temp_enabled,
           self.total_count,
           self.current_count, self.overcurrent_faults,
           self.voltage_count, self.undervoltage_faults, self.overvoltage_faults,
           self.temperature_count, self.overheat_faults,
           self.no_measurements)
        return outstr

################################################################################


class Device:
    """
    class represents one virtual device
    """

    def __init__(self, device_name, disable_current=False,
                 disable_voltage=False, disable_temperature=False):
        self.__device_name = device_name
        self.measurements = Measurements(device_name, not disable_temperature,
                                         not disable_voltage,
                                         not disable_current)
        self.__disable_current = disable_current
        self.__disable_voltage = disable_voltage
        self.__disable_temperature = disable_temperature

    def __generate_current_measurement(self):
        if self.__disable_current:
            return None

        if random_boolean() is False:
            return None

        value = random.randint(0, 1000) / 100.0
        fault = ""

        if value > 9.0:
            fault = "overcurrent"
            self.measurements.overcurrent_faults += 1

        record = {
            "value": value,
            "unit": "A",
            "fault": fault
        }

        return record

    def __generate_voltage_measurement(self):
        if self.__disable_voltage:
            return None

        if random_boolean():
            return None

        value = random.randint(2100, 2500) / 10.0

        fault = ""

        if value > 245:
            fault = "overvoltage"
            self.measurements.overvoltage_faults += 1
        elif value < 215:
            fault = "undervoltage"
            self.measurements.undervoltage_faults += 1

        record = {
            "value": value,
            "unit": "V",
            "fault": fault
        }

        return record

    def __generate_temperature_measurement(self):
        if self.__disable_temperature:
            return None

        if random_boolean() is False:
            return None

        value = random.randint(0, 1000) / 10.0
        fault = ""

        if value > 90.0:
            fault = "overheat"
            self.measurements.overheat_faults += 1

        record = {
            "value": value,
            "unit": "C",
            "fault": fault
        }

        return record

    def generate_message(self):
        """
        Method randomly generates random messages
        """
        if random_boolean() is False:
            return None

        self.measurements.total_count += 1

        utc_time = time.time()
        fraction_part = str((utc_time % 1) * 1000000).split('.')[0]
        message = {
            "name": self.__device_name,
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S.{0}UTC"
                                       .format(fraction_part), time.gmtime(utc_time))
        }

        entry_inserted = False

        measurement = self.__generate_current_measurement()
        if measurement is not None:
            self.measurements.current_count += 1
            message["current"] = measurement
            entry_inserted = True

        measurement = self.__generate_temperature_measurement()
        if measurement is not None:
            self.measurements.temperature_count += 1
            message["temperature"] = measurement
            entry_inserted = True

        measurement = self.__generate_voltage_measurement()
        if measurement is not None:
            self.measurements.voltage_count += 1
            message["voltage"] = measurement
            entry_inserted = True

        if not entry_inserted:
            self.measurements.no_measurements += 1

        return message

################################################################################


class Application:
    """
    this class represents whole application
    """

    def __init__(self,
                 max_random_devices=100,
                 runtime=10,
                 timeout=0.001,
                 address="127.0.0.1",
                 port=50000,
                 device_names=None):

        self.__address = address
        self.__devices = []
        self.__connection = http.client.HTTPConnection(address, port)
        self.__start_time = time.time()
        self.__runtime = runtime
        self.__timeout = timeout

        if device_names is None:
            for _ in range(random.randint(1, max_random_devices)):
                random_device_name = ''.join(random.SystemRandom().choice(
                    string.ascii_letters + string.digits) for _ in range(10))
                self.__devices.append(Device(random_device_name,
                                             random_boolean(), random_boolean(),
                                             random_boolean()))
                print(">>> added device:", random_device_name)
        else:
            for devname in device_names:
                self.__devices.append(Device(devname, random_boolean(),
                                             random_boolean(),
                                             random_boolean()))
                print(">>> added device:", devname)

    def run(self):
        """
        execute simulator
        """
        while True:
            for _ in self.__devices:
                message = _.generate_message()
                time.sleep(self.__timeout)
                if message is not None:
                    # print(message)
                    self.__send_message(message)

            if (time.time() - self.__start_time) >= self.__runtime:
                break

        self.__connection.close()

    def __send_message(self, message):
        self.__connection.request(
            method="POST",
            url="/device/measurement",
            body=json.dumps(message),
        )
        response = self.__connection.getresponse()

        if response.status != 200:
            print(response.status, response.reason)

    def print_results(self):
        """
        method will print all totals
        """
        grand_total = 0
        for _ in self.__devices:
            print(_.measurements)
            grand_total += _.measurements.total_count
        print("GRAND TOTAL", grand_total)

        time.sleep(2) # insert little timeout before we get complete results

        print()
        print("<<<<< GET /device/results >>>>>")

        self.__connection.request(
            method="GET",
            url="/device/results"
        )
        response = self.__connection.getresponse()

        print(str(response.read().decode('ascii')))

        if response.status != 200:
            print(response.status, response.reason)

################################################################################


APP = Application(
    device_names=["device-1", "device-2", "device-3", "device-4"])

#APP = Application()

APP.run()
APP.print_results()
