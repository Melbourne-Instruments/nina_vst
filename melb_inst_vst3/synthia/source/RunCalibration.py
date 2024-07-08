import traceback
from elkpy import sushicontroller as sc
import numpy as np
import time
from datetime import datetime
import subprocess
from enum import Enum
import RPi.GPIO as GPIO    

import sys


class CalOption(Enum):
    DEFAULT = 1
    TUNEUP = 2
    FACTORY=3
    CHECKTEMP=4
    MIXVCA=5
    FILTER=6

#make sure that the disk is in RW mode
subprocess.run(
        ["elk_system_utils", "--remount-as-rw"])

# arguments
n = len(sys.argv)
print("start")
print("Total arguments passed:", n)
cal_option = CalOption.DEFAULT
if(n > 1):
    val = sys.argv[1]
    print(val)

    if("tuneup" in sys.argv[1]):
        cal_option = CalOption.TUNEUP
        print("fast tune")
    if("factory-cal" in sys.argv[1]):
        cal_option = CalOption.FACTORY
        print("factory tune")
    if("check-temp" in sys.argv[1]):
        cal_option = CalOption.CHECKTEMP
        print("check temp")
    if("vca-mix" in sys.argv[1]):
        cal_option = CalOption.MIXVCA
        print("vca mix")
    if("filter" in sys.argv[1]):
        cal_option = CalOption.FILTER
        print("cal filter")


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
            self.params = []
            self.temp_data = []
            self.start_time = datetime.now()
            for process in self.processors:
                print(process)
                if process.name == 'ninavst':
                    self.nina_process = process.id
                    print(self.nina_process)
            nina_params = self.controller.parameters.get_processor_parameters(
                self.nina_process)

            for param in nina_params:
                value = self.controller.parameters.get_parameter_value(
                    self.nina_process, param.id)
                if 'Run Tuning' in param.name:
                    self.params.append(make_voice_param(
                        param.name, param.id, value))
                if 'Main Vca Cal' in param.name:
                    self.params.append(make_voice_param(
                        param.name, param.id, value))
                if 'Mix Vca Cal' in param.name:
                    self.params.append(make_voice_param(
                        param.name, param.id, value))
                if 'Filter Cal' in param.name:
                    self.params.append(make_voice_param(
                        param.name, param.id, value))
                if 'Write Temps' in param.name:
                    self.params.append(make_voice_param(
                        param.name, param.id, value))
                if 'Reload Cal' in param.name:
                    self.params.append(make_voice_param(
                        param.name, param.id, value))
        except Exception as ex:
            template = "Error: An exception of type '{0}' occurred:\n{1!r}"
            msg = template.format(type(ex).__name__, ex.args)
            print(msg)
            print(traceback.format_exc())
            self.controller.close()
            raise(ex)

    def getParam(self, find_name):
        return [obj for obj in self.params if obj.name == find_name][0]

    def setTuning(self, run):
        param = self.getParam('Run Tuning')
        set_val = 0.0
        if run:
            set_val = 1.0

        self.controller.parameters.set_parameter_value(3, param.id, set_val)

    def setMainVca(self, run):
        param = self.getParam('Main Vca Cal')
        set_val = 0.0
        if run:
            set_val = 1.0
        self.controller.parameters.set_parameter_value(3, param.id, set_val)

    def setMixVca(self, run):
        param = self.getParam('Mix Vca Cal')
        set_val = 0.0
        if run:
            set_val = 1.0
        self.controller.parameters.set_parameter_value(3, param.id, set_val)

    def setFilter(self, run):
        param = self.getParam('Filter Cal')
        print(self.params)
        print(param)
        set_val = 0.0
        if run:
            set_val = 1.0

        print(param.id)
        print(set_val)
        self.controller.parameters.set_parameter_value(3, param.id, set_val)

    def setWriteTemp(self, run):
        param = self.getParam('Write Temps')
        set_val = 0.0
        if run:
            set_val = 1.0
        self.controller.parameters.set_parameter_value(3, param.id, set_val)

    def reloadCal(self, run):
        param = self.getParam('Reload Cal')
        set_val = 0.0
        if run:
            set_val = 1.0
        self.controller.parameters.set_parameter_value(3, param.id, set_val)

    def LogCalInfo(self):
        self.setWriteTemp(True)
        time.sleep(1)
        self.setWriteTemp(False)
        file = "/udata/nina/tuning/cal_info.dat"
        array = np.fromfile(file, dtype="<f")
        print(array[0])
        return array

    def isTestDone(self):
        array = self.LogCalInfo()
        value = array[12]
        print("test running ", value, value < 0.5)
        return value < 0.5

    def LogTemp(self):

        self.setWriteTemp(True)
        time_t = datetime.now() - self.start_time
        time_t = time_t.total_seconds()
        time.sleep(1)
        self.setWriteTemp(False)
        file = "/udata/nina/tuning/cal_info.dat"
        array = np.fromfile(file, dtype="<f")
        array = np.append(array, time_t)
        self.temp_data.append(array)
        


    def checkTuningDelta(self):
        array_mean = np.array(self.temp_data)
        array_mean = array_mean[:, 0::11]
        array_mean = np.mean(array_mean, 1)
        return array_mean[0] - array_mean[-1]

    def currentRateOfChange(self):
        array = np.array(self.temp_data)
        print("size", np.size(array))
        if(np.size(array) > 14):

            array_mean = array[:, 0::11]
            array_mean = np.mean(array_mean, 1)
            t_d = array[-1, 13] - array[-2, 13]
            y_d = array_mean[-1] - array_mean[-2]
            print("deltas ", t_d, y_d)
            return y_d/t_d
        return 1/500


def getSystemTemp():
    with open('/sys/class/thermal/thermal_zone0/temp') as f:
        temp = f.read()
        return int(temp) / 1000
p = interface()

####STARTUP####

if(cal_option == CalOption.DEFAULT):
    base_temp = getSystemTemp()
    print("base temp: "+ str(base_temp))
    #base_temp = 19
    print("full user cal")
    # sleep at the start to make sure osc are up
    start_time = datetime.now()
    time.sleep(2)
    array = p.LogCalInfo()
    array_size = np.size(array)
    print(array_size)
    array_temps = array[:-1]
    med_temp = np.median(array_temps)
    array_temps_diff = array_temps[:] - med_temp
    print(array_temps_diff)
    if(np.max(np.abs(array_temps_diff)) > .2):
        print("osc not started?")

    i = 0
    tuning_delta_enough = False
    time_taken = 0
    other_cal_run = False
    print("time taken", time_taken)
    start_time = datetime.now()
    heating_complete = False

    # do 1 big tuning chunk at the start
    while((time_taken < 60*8)):
        time_taken = (datetime.now() - start_time).total_seconds()
        print("time so far: ", time_taken)
        p.setTuning(True)
        time.sleep(30)
        p.setTuning(False)

        time.sleep(10)

    while((not heating_complete)):
        
        temp = getSystemTemp()
        temp_d = temp - base_temp
        print("temps: " + str(temp) + " " + str(temp_d))
        if ((float(temp_d) > 18) | (float(temp) > 55.0)):
            heating_complete = True
            print("heating complete " + str(time_taken))
            
        time_taken = (datetime.now() - start_time).total_seconds()
        print("time so far: "+  str(time_taken) + " "  + str(temp_d))
        p.setTuning(True)
        time.sleep(20)
        p.setTuning(False)
        i += 1
        time.sleep(80)
    
    heating_complete = False
     
    # Run the main vca test and stop when complete
    print("run main vca")
    p.setMainVca(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    time.sleep(5)
    p.setMainVca(False)
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))

    # run the mix vca cal and stop when complete
    print("run mix vca")
    p.setMixVca(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setMixVca(False)
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))
    
    #2nd stage of heating
    while((not heating_complete)):
        
        temp = getSystemTemp()
        temp_d = temp - base_temp
        print("temps: " + str(temp) + " " + str(temp_d))
        if ((float(temp_d) > 27) | (float(temp) > 68.0)):
            heating_complete = True
            print("heating complete " + str(time_taken))
            
        time_taken = (datetime.now() - start_time).total_seconds()
        print("time so far: "+  str(time_taken) + " "  + str(temp_d))
        p.setTuning(True)
        time.sleep(30)
        p.setTuning(False)
        i += 1
        time.sleep(80)
    

    # run the filter vca cal and stop when complete
    print("run filter ")
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))

    # run the filter cal script
    print("run analysis")
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/filter_tuning_routine.py"])
    print("run filter")
    other_cal_run = True
    
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))

    # run the osc tuning routine and reload the cal. this way the last tune cycle has better coverage of the test space as its model is already more correct
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/osc_tuning_routine.py"])
    time.sleep(5)
    p.reloadCal(True)
    
    
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))
    
    # run last tuning at the end
    time.sleep(5)
    for i in range(10):
        p.setTuning(True)
        time.sleep(60)
        p.setTuning(False)
        time.sleep(10)

    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/osc_tuning_routine.py"])
    time.sleep(5)

    time_taken = (datetime.now() - start_time).total_seconds()
    print("time so far: "+  str(time_taken) + " "  + str(temp_d))
    # reload the cal in preperation for testing
    p.reloadCal(True)
    time.sleep(3)
    
    #run the filter cal again as it seems to require another tune
    print("run filter ")
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)

    # run the filter cal script
    print("run analysis")
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/filter_tuning_routine.py"])
    print("run filter")
    
    
    p.reloadCal(True)
    time.sleep(1)
    np.save('/udata/temp_data.npy', p.temp_data)

    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))
    time_taken = (datetime.now() - start_time).total_seconds()
    print("time so far: "+  str(time_taken) + " "  + str(temp_d))
    time.sleep(1)

# short single stage tune
if(cal_option == CalOption.TUNEUP):

    print("run fast tune")
    # sleep at the start to make sure osc are up
    start_time = datetime.now()
    time.sleep(2)
    array = p.LogCalInfo()
    array_size = np.size(array)
    print(array_size)
    array_temps = array[:-1]
    med_temp = np.median(array_temps)
    array_temps_diff = array_temps[:] - med_temp
    print(array_temps_diff)
    if(np.max(np.abs(array_temps_diff)) > .2):
        print("osc not started?")

    i = 0
    tuning_delta_enough = False
    current_rate_of_change = 1/20000
    time_taken = 0
    other_cal_run = False
    print("time taken", time_taken)
    start_time = datetime.now()

    # run tuning for 4 min
    while((time_taken < 60*4)):
        time_taken = (datetime.now() - start_time).total_seconds()
        print("time so far: ", time_taken, tuning_delta_enough, i)
        p.setTuning(True)
        time.sleep(10)
        p.setTuning(False)

        time.sleep(10)

    #run the tuning analysis
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/osc_tuning_routine.py"])
    time.sleep(5)

    # reload the cal in preperation for testing
    p.reloadCal(True)
    
    
    
if(cal_option == CalOption.CHECKTEMP):
    base_temp = getSystemTemp()
    print("base temp: "+ str(base_temp))
    
    #pi starts throttling at 80. subtract 2 for error margin 
    max_temp = 80 - 2
    max_startup_temp = max_temp - 35
    if(base_temp > max_startup_temp):
        print("startup temp is too high")
        sys.exit(1)

if(cal_option == CalOption.FACTORY):
    base_temp = getSystemTemp()
    print("base temp: "+ str(base_temp))
    
    #pi starts throttling at 80. subtract 2 for error margin 
    max_temp = 80 - 2
    max_startup_temp = max_temp - 35
    if(base_temp > max_startup_temp):
        print("startup temp is too high")
        sys.exit(1)
    #base_temp = 19
    print("factory cal")
    # sleep at the start to make sure osc are up
    start_time = datetime.now()
    time.sleep(2)
    array = p.LogCalInfo()
    array_size = np.size(array)
    print(array_size)
    array_temps = array[:-1]
    med_temp = np.median(array_temps)
    array_temps_diff = array_temps[:] - med_temp
    print(array_temps_diff)
    if(np.max(np.abs(array_temps_diff)) > .2):
        print("osc not started?")

    i = 0
    tuning_delta_enough = False
    time_taken = 0
    other_cal_run = False
    print("time taken", time_taken)
    start_time = datetime.now()
    heating_complete = False

    # do 1 big tuning chunk at the start
    while((time_taken < 60*5)):
        time_taken = (datetime.now() - start_time).total_seconds()
        print("time so far: ", time_taken)
        p.setTuning(True)
        time.sleep(30)
        p.setTuning(False)

        time.sleep(10)

    while((not heating_complete)):
        
        temp = getSystemTemp()
        temp_d = temp - base_temp
        print("temps: " + str(temp) + " " + str(temp_d))
        if ((float(temp_d) > 18) | (float(temp) > 55.0)):
            heating_complete = True
            print("heating complete " + str(time_taken))
            
        time_taken = (datetime.now() - start_time).total_seconds()
        print("time so far: "+  str(time_taken) + " "  + str(temp_d))
        p.setTuning(True)
        time.sleep(20)
        p.setTuning(False)
        i += 1
        time.sleep(80)
    
    heating_complete = False
     
    # Run the main vca test and stop when complete
    print("run main vca")
    p.setMainVca(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    time.sleep(5)
    p.setMainVca(False)
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))

    # run the mix vca cal and stop when complete
    print("run mix vca")
    p.setMixVca(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setMixVca(False)
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))
    
    #2nd stage of heating
    while((not heating_complete)):
        
        temp = getSystemTemp()
        temp_d = temp - base_temp
        print("temps: " + str(temp) + " " + str(temp_d))
        if ((float(temp_d) > 35) | (float(temp) > 68.0)):
            heating_complete = True
            print("heating complete " + str(time_taken))
            
        time_taken = (datetime.now() - start_time).total_seconds()
        print("time so far: "+  str(time_taken) + " "  + str(temp_d))
        p.setTuning(True)
        time.sleep(30)
        p.setTuning(False)
        i += 1
        time.sleep(80)
    

    # run the filter vca cal and stop when complete
    print("run filter ")
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))

    # run the filter cal script
    print("run analysis")
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/filter_tuning_routine.py"])
    print("run filter")
    other_cal_run = True
    
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))

    # run the osc tuning routine and reload the cal. this way the last tune cycle has better coverage of the test space as its model is already more correct
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/osc_tuning_routine.py"])
    time.sleep(5)
    p.reloadCal(True)
    
    
    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))
    
    # run last tuning at the end
    time.sleep(5)
    for i in range(5):
        p.setTuning(True)
        time.sleep(60)
        p.setTuning(False)
        time.sleep(10)

    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/osc_tuning_routine.py"])
    time.sleep(5)

    time_taken = (datetime.now() - start_time).total_seconds()
    print("time so far: "+  str(time_taken) + " "  + str(temp_d))
    # reload the cal in preperation for testing
    p.reloadCal(True)
    time.sleep(3)
    
    #run the filter cal again as it seems to require another tune
    print("run filter ")
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)

    # run the filter cal script
    print("run analysis")
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/filter_tuning_routine.py"])
    print("run filter")
    
    
    p.reloadCal(True)
    time.sleep(1)
    np.save('/udata/temp_data.npy', p.temp_data)

    temp = getSystemTemp()
    temp_d = temp - base_temp
    print("temps: " + str(temp) + " " + str(temp_d))
    time_taken = (datetime.now() - start_time).total_seconds()
    print("time so far: "+  str(time_taken) + " "  + str(temp_d))
    time.sleep(1)
    
    #pulse the output to turn off the analog section
    GPIO.setwarnings(False) 
    GPIO.setmode(GPIO.BCM)    
    GPIO.setup(17, GPIO.OUT, initial=GPIO.LOW)
    time.sleep(0.010)
    GPIO.setup(17, GPIO.IN)
    

if(cal_option == CalOption.MIXVCA):
    
    # run the mix vca cal and stop when complete
    print("run mix vca")
    p.setMixVca(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setMixVca(False)
    

if(cal_option == CalOption.FILTER):
    
    # run the filter vca cal and stop when complete
    time.sleep(1)
    print("run filter ")
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
        
    p.setFilter(False)
    time.sleep(2)
    p.setFilter(True)
    test_done = False
    while(not test_done):
        time.sleep(5)
        test_done = p.isTestDone()
    p.setFilter(False)
    time.sleep(2)
    
    # run the filter cal script
    print("run analysis")
    subprocess.run(
        ["python3", "/home/root/nina/synthia_vst.vst3/Contents/filter_tuning_routine.py"])
    
    time.sleep(1)
    p.reloadCal(True)
    time.sleep(1)
    
#make rootfs back to read only
subprocess.run(
        ["elk_system_utils", "--remount-as-ro"])

print("done")
