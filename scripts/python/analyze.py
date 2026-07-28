import sys
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt

def o_n(n):
    return n
def o_nlogn(n):
    return n * np.log(n)
def o_nloglogn(n):
    return n * np.log(np.log(n))

def main():
    path = Path(sys.argv[1])
    dirs = [item.name for item in path.iterdir() if item.is_dir()]
    for i in range(len(dirs)):
        print(i, dirs[i])
    which = int(input('which?'))

    data = {}
    algs = []
    with open('./results/' + dirs[which] + '/results.txt', 'r') as file:
        for line in file:
            line = line.strip()

            parts = line.split('|')
            newParts = []
            for i in range(len(parts)):
                tmp = parts[i].strip()
                if len(tmp) != 0:
                    newParts.append(tmp)

            if len(newParts) >= 2 and newParts[0] == "Distribution" and len(algs)==0:
                for i in range(2, len(newParts)):
                    algs.append(newParts[i])


            if len(newParts) >= 2 and newParts[1].isdigit():
                if newParts[0] not in data:
                    data[newParts[0]] = {}
                if int(newParts[1]) not in data[newParts[0]]:
                    data[newParts[0]][int(newParts[1])] = []

                tmp = []
                for i in range(2,len(newParts)):
                    tmp.append(float(newParts[i].strip('*')))

                data[newParts[0]][int(newParts[1])].append(tmp)

    colors = ['r','g','b','c']

    for dist in data:
        data_dist = data[dist]
        print(dist)

        x = []
        y = []
        y_low = []
        y_up = []
        for size in data_dist:
            x.append(size)

            data_dist_size = data_dist[size]
            mean_vals = np.mean(data_dist_size, axis=0)
            std_vals = np.std(data_dist_size, axis=0)

            print("   ", size)
            print("      ", mean_vals)

            y.append(mean_vals)
            y_up.append(mean_vals + std_vals)
            y_low.append(mean_vals - std_vals)

        x = np.log(np.array(x))
        y = np.log(np.array(y))
        y_low = np.log(np.array(y_low))
        y_up = np.log(np.array(y_up))

        plt.figure(figsize=(8, 5))
        for i in range(y.shape[1]):
            plt.plot(x, y[:,i], marker='.', color=colors[i], label='Average (Mean) alg '+algs[i])
            plt.fill_between(
                x,
                y_low[:,i],
                y_up[:,i],
                color=colors[i],
                alpha=0.2,
            )

        plt.xlabel('log N')
        plt.ylabel('log T')
        plt.title('Distribution ' + str(dist))
        plt.legend()
        plt.show()

    o_functions = [o_n, o_nlogn, o_nloglogn]

    for dist in data:
        data_dist = data[dist]
        print(dist)

        sizes = []
        num_exp = 9999
        for size in data_dist:
            sizes.append(size)
            num_exp = min(num_exp, len(data_dist[size]))

        print(sizes)
        print(num_exp)

        for i in range(len(algs)):
            plt.figure(figsize=(8, 5))
            for o_fun in o_functions:
                v = []
                for exp in range(num_exp):
                    for size1 in data_dist:
                        for size2 in data_dist:
                            if size1 >= 1000000 and size1 < size2:
                                tmp = np.abs((o_fun(size1))/(o_fun(size2)) - data_dist[size1][exp][i]/data_dist[size2][exp][i])
                                if tmp>0.1:
                                    tmp = 0.1
                                v.append(tmp)

                plt.hist(v, bins=50, alpha=0.25, label=str(o_fun.__name__))
            plt.legend()
            plt.title('Distribution ' + str(dist) + ' alg ' + algs[i])
            plt.show()

if __name__ == "__main__":
    main()
