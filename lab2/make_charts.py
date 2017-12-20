"""Make charts from run-benchmarks.sh script"""

import csv
import matplotlib.pyplot as plt
import numpy as np


def read_csv(filename):
    """Read the benchmark data into a dicitonary"""
    csv_data = []
    with open(filename) as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            csv_data.append({'npages': int(row[0]),
                             'nframes': int(row[1]),
                             'method': row[2],
                             'program': row[3],
                             'num_page_faults': int(row[4]),
                             'num_disk_reads': int(row[5]),
                             'num_disk_writes': int(row[6])})
    return csv_data


def plot_graph(data):
    method = data[0]['method']
    program = data[0]['program']
    npages = data[0]['npages']

    x = [d['nframes'] for d in data]
    y_faults = [d['num_page_faults'] for d in data]
    y_reads = [d['num_disk_reads'] for d in data]
    y_writes = [d['num_disk_writes'] for d in data]

    plt.plot(x, y_faults, 'go-', label='Page faults')
    plt.plot(x, y_reads, 'r^--', label='Disk reads')
    plt.plot(x, y_writes, 'b+:', label='Disk writes')

    plt.xlabel('Number of physical memory frames')
    plt.ylabel('Number of events')
    plt.title("{} with {} replacement and {} pages".format(
        program, method, npages))
    plt.legend()

    filename = "charts/{}_{}_{}.png".format(
        method, program, npages)

    plt.savefig(filename)
    plt.clf()


csv_data = read_csv('benchmarks.csv')
csv_data = np.reshape(csv_data, (9, 10))


for d in csv_data:
    plot_graph(d)
