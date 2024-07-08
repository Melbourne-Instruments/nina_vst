import traceback
from scipy.optimize import curve_fit
import numpy as np
from scipy.sparse import data
import platform
import time

model_list_2 = np.array([])
model_list_1 = np.array([])
num = 0
first = False
data_array_all = np.empty([0, 6])

files = [
"voice_0_osc_0", "voice_0_osc_1",
"voice_1_osc_0","voice_1_osc_1",
"voice_2_osc_0","voice_2_osc_1",
"voice_3_osc_0","voice_3_osc_1",
"voice_4_osc_0","voice_4_osc_1",
"voice_5_osc_0","voice_5_osc_1",
"voice_6_osc_0","voice_6_osc_1",
"voice_7_osc_0","voice_7_osc_1",
"voice_8_osc_0","voice_8_osc_1",
"voice_9_osc_0","voice_9_osc_1",
"voice_10_osc_0","voice_10_osc_1",
"voice_11_osc_0","voice_11_osc_1",
]

avepop1 = {0,0,0,0,0,0,0,0,0,0,0}
avepop2 = {0,0,0,0,0,0,0,0,0,0,0}
avepop1 = np.zeros((11,1))
avepop2 = np.zeros((11,1))
avecount = 0
dir = "/udata/nina/tuning"
dir_save = "/udata/nina/calibration"

#run the calibration for each osc
for file in files:
   
   #initial models for osc up and osc down. this helps the cal run faster 
    popt1 = -1.93283460e-02,  5.00115281e-02,  0.00000000e+00, -1.21517057e-08,1.92683254e-07,  1.28621805e-04,  3.08219507e-06, -7.41912736e-05,3.34000000e-21,  1.79884330e+11,  1.00000000e+0

    popt2 = -1.91826149e-02,  5.20942994e-02,  0.00000000e+00, 2.60219116e-07,7.92738256e-08,  4.89865412e-05,  1.80711854e-06, -5.27539709e-05,-4.44000000e-21, -9.04849644e+10,  1.00000000e+00 
        
    try:
        model_list_2 = np.array([])
        model_list_1 = np.array([])
        num = 0
        first = False
        data_array_all = np.empty([0, 6])
        data_array_in = np.array([])
        with open(dir+"/" + file + ".txt", 'r') as filestream:
            print(file)
            for line in filestream:
                if line !='\n':
                    line_items = (line.split(","))
                    line_nums = np.array(line_items).astype(float)
                    line_nums = np.concatenate((line_nums, [0, 0]))
                    data_array_in = np.concatenate((data_array_in, line_nums))

            #filter the data mesurements for values over 80kHz. seems to improve the consistancy & reliability of the models
            max_freq = 80000
            data_array_in = np.reshape(data_array_in, (-1, 6))
            test1 = data_array_in[0::2,2]
            thresh = 1/max_freq
            filter = (data_array_in[0::2,2] > thresh ) & (data_array_in[0::2,3] >thresh  ) & (data_array_in[1::2,2] >thresh ) & (data_array_in[1::2,3] > thresh )
            data_array_in = data_array_in[np.repeat(filter,2),:]
            
            shape = np.shape(data_array_in)
            data_array_in = data_array_in[int(shape[0]/8):-1,:]
            data_array_all = np.append(
                data_array_all[:], data_array_in[:, :], axis=0)
            file
            printing = (1/(data_array_all[:,2] + data_array_all[:,3]))
            
            #model is ran on the log2 of the freq, since this is how we generate the signals in the model
            data_array_all[:,2] = np.log2(1/data_array_all[:,2])
            data_array_all[:,3] = np.log2(1/data_array_all[:,3])


            freq = 96000

            mul = 0
            samples = 4
            shape_d = np.shape(data_array_all)
            array = np.zeros((shape_d[0],1))
            error_filter_1 = array[:,0] <1
            error_filter_2 = error_filter_1.copy()
            steps = int(shape_d[0]/samples)
            counter = 0
            func2_array = {0,0,0,0,0,0,0,0,0,0,1}

            def func(X, a, b, c, d, e, f, g, h, i, j, k):
                    # put in coeffs for 50-50 tune
                    f1 = X[0]
                    f2 = X[1]

                    t = X[2]


                    out =  e* (2**f2) +t + 0.001/(f1 + j) +(a*t + b)*f1+ (f)*(f2) + c + d* (2**f1) + h*(((f1)*1)**2) + g*(((f1)*1)**3) + 0*i*(((f1)*1)**2)
                    return out

            maxcount =101
            
            
            start_time = time.time()
            while(counter < maxcount):
                counter += 1

                def func3(X3, m, c):
                    return m*X3 + c
                datatest = data_array_all[error_filter_1,2]
                
                
                if(counter >1):
                    popt1, pcov = curve_fit(
                        func, (data_array_all[(error_filter_1), 2], data_array_all[(error_filter_1), 3], data_array_all[(error_filter_1), 4]), (data_array_all[(error_filter_1), 0]),p0=popt1, maxfev=1000000)

                    popt2, pcov = curve_fit(
                        func, (data_array_all[error_filter_2,3], data_array_all[error_filter_2, 2], data_array_all[error_filter_2, 5]), (data_array_all[error_filter_2, 1]),p0 = popt2, maxfev=1000000)
                    if counter==maxcount:
                        popt1[2] = 0

                yline1 = func(
                    (data_array_all[:, 2], data_array_all[:, 3], data_array_all[:, 4]), *popt1)
                if counter==maxcount:
                    print()
                error1 = (data_array_all[:, 0] - yline1)

                if(counter < maxcount-2):
                    for i in range(steps+1):
                        datay = error1[(i * samples): ((i * samples) + samples)]

                        data_array_all[(i * samples): ((i * samples) + samples),
                                    4] += (np.mean(datay) + popt1[2])
                if counter == maxcount:
                        popt2[2] = 0
                yline2 = func(
                (data_array_all[:, 3], data_array_all[:, 2], data_array_all[:, 5]), *popt2)

                if counter == maxcount:
                    print()
                error2 = (data_array_all[:, 1] - yline2)
                if counter < maxcount-2:
                    for i in range(steps+1):
                        datay = error2[(i * samples): ((i * samples) + samples)]

                        data_array_all[(i * samples): ((i * samples) + samples),
                                    5] += (np.mean(datay) + popt2[2])
                        
                #after several iterations, we remove badly fitting values. this seems to dramatically improve the quality of the cal. As we iterate, we further filter more values
                if counter == int( maxcount/2):
                    print()
                    error_filter_1 = np.abs(error1[:]) < .01
                    #print([error_filter_1[:]==False])
                    #data_array_all[error_filter_1, 0] -= (error1[error_filter_1])
                    error_filter_2 = np.abs(error2[:]) < .01
                    #data_array_all[error_filter_2, 1] -= (error2[error_filter_2])
                    #print(error_filter_2[error_filter_2[:] == False])
                if counter == maxcount/1.5:
                    error_filter_1 = np.abs(error1[:]) < 0.005
                    #data_array_all[error_filter_1, 0] -= (error1[error_filter_1])
                    error_filter_2 = np.abs(error2[:]) < 0.005
                    #data_array_all[error_filter_2, 1] -= (error2[error_filter_2])
                    print()
                    


                num += 1
            elapsed_time = time.time() - start_time

            #print(data_array_all[:,4])
            #print (data_array_all[:,5])

            print("models osc up\n")
            print("{", end="")
            for num in popt1[:-1]:
                print(" {:.2e} ".format( num ))

            avepop1 = np.add(avepop1,np.array(popt1[:]))
            avepop2 = np.add(avepop2,np.array(popt2[:]))
            avecount +=1

            print("models osc down\n")
            print("{", end="")
            for num in popt2[:-1]:
                print(" {:.2e} ".format( num ))
            print(popt2[-1], end="};\n\n")

            with open(dir_save+"/" + file+".model", 'w') as f:
                for num in popt1[:-1]:
                    f.write(str(num) + ' ')
                f.write('\n')
                for num in popt2[:-1]:
                    f.write(str(num) + ' ')
                f.close()
            print("wrote to" + dir + file)
            print(avepop1/avecount)
            print(avepop2/avecount)
            print (elapsed_time)
    except:
        print("tuning failed 2 ")
        traceback.print_exc()
    

