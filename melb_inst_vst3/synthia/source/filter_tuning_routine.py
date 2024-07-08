from scipy.optimize import curve_fit
import numpy as np
from scipy.sparse import data


num_voices =12
def func(X, a, b, c,d,e,f,g):
    return X[0]*(a + -0.102195*X[1])  + c + .8026*X[1] + 0*np.tan(e*X[0])
sumg = 0
allx = []
allt = []
ally = []
allpopt = []

for voicen in range(12):
    files = []
    filestring  = "/udata/nina/tuning/voice_" + str(voicen) + "_filter.dat"
    files.append(filestring)
    print( filestring)
    stims = ([])
    temps = []
    resf = []
    base_temp = -100
    for file in files:
        array = np.fromfile(file, dtype="<f")
        temp = array[0]
        print(temp)
        array = array[1:]
        array = array.reshape([2,-1])
        shapea = array.shape
        t_array = temp*np.ones((1,shapea[1]))
        array = np.concatenate((array, t_array)) 
        jumps = np.diff(array[1,:])
        jumps = np.absolute(jumps)[:] > 0.001
        starts = np.array((np.where(jumps==True)))
        starts = starts[0,:]
        start = 0
        for value in starts:
            result2 = np.fft.fft(array[0,start:value])
            sample_rate = 1/96000
            freqs = np.fft.fftfreq(result2.shape[0],sample_rate)
            q = np.absolute(result2)
            q = q[freqs[:] > 0]
            resf.append( freqs[q.argmax(axis=0)])
            stims.append(np.median(array[1,start:value]))
            if(base_temp == -100):
                base_temp = temp
            temps.append(temp - base_temp)
            start = value+1
        
    resf = np.array(resf)
    stims = np.array(stims)
    temps = np.array(temps)
    stims = stims[(resf < 2000) & (resf > 50)]
    temps = temps[(resf < 2000) & (resf > 50)]
    resf = resf[(resf < 2000) & (resf > 50)]
    
    trim_f = resf[resf[:] < 200000]
    trim_s = stims[resf[:] < 200000]
    trim_t= temps[resf[:] < 200000]
    
    allx = np.append(allx, trim_s)
    allt = np.append(allt, trim_t)
    popt0  = 1,1,1,1,np.pi/2,1,1
   
    #linear model max freq, in practice, the filter will not reach this value 
    freq_max = 40000
    
    #middle freq, input of zero should give an FC of this value
    freq_middle = 1000
    resf_log = (np.log2(resf) -np.log2(freq_middle))/ (np.log2(freq_max) - np.log2(freq_middle))
    popt1, pcov = curve_fit(func, (resf_log, temps), stims,p0 = popt0 ,maxfev=1000000)
    allpopt.append(popt1)
    ally = np.append(ally, np.log2(trim_f) )
    print(str(popt1[0]) + "  " + str(popt1[3]))
    print(popt1)
    print(base_temp)
    with open("/udata/nina/calibration/" + "voice_" +str(voicen) +"_filter.model", 'w') as f:
        f.write(str(popt1[0]) + ' ')
        f.write(str(popt1[2]) + ' ')
        f.write(str(base_temp) + ' ')
    y_est = func((resf_log,temps), *popt1)
    error = y_est - stims
    sumg += popt1[0]
    line_count = 0
    
print("done")