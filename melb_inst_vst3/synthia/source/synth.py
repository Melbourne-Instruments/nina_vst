import traceback
from elkpy import sushicontroller as sc
import numpy as np


class voice_param(object):
    def __init__(self, name, param_id, value):
        self.name = name
        self.id = param_id
        self.value = value

    def __str__(self):
        return f'{self.name}, {self.id}, {self.value}'

    def __repr__(self):
        return f'[{self.name}, {self.id}, {self.value}]'


def make_voice_param(name, param_id, value):
    param = voice_param(name, param_id, value)
    return param


class interface(object):
    def __init__(self):
        self.controller = sc.SushiController()
        try:
            self.sushi_version = self.controller.system.get_sushi_version()
            self.processors = self.controller.audio_graph.get_all_processors()
            self.nina_process = None
            self.sampler_process = None
            self.sampler_trigger = None
            for process in self.processors:
                print(process)
                if process.name == 'synthiavst':
                    self.nina_process = process.id
                    print (self.nina_process)

        except Exception as ex:
            template = "Error: An exception of type '{0}' occurred:\n{1!r}"
            msg = template.format(type(ex).__name__, ex.args)
            print(msg)
            print(traceback.format_exc())
            self.controller.close()
            raise(ex)

    def set_config(self):
      self.controller.parameters.set_parameter_value(1, 224,1)
      self.controller.parameters.set_parameter_value(1, 228,1)
      self.controller.parameters.set_parameter_value(1, 52,.5)
      self.controller.parameters.set_parameter_value(1, 168,1)
      self.controller.parameters.set_parameter_value(1, 51,0.6)
      self.controller.parameters.set_parameter_value(1, 55,0.6)
      self.controller.parameters.set_parameter_value(1, 10,1)
      self.controller.parameters.set_parameter_value(1, 7,0.5)
      self.controller.parameters.set_parameter_value(1, 66,1)
      self.controller.parameters.set_parameter_value(1, 57,1)
      self.controller.parameters.set_parameter_value(1, 74,1)
      self.controller.parameters.set_parameter_value(1, 72,0)
      self.controller.parameters.set_parameter_value(1, 75,0)
      print('sent changes')



p = interface()
p.set_config()
