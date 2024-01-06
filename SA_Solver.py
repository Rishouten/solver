import json
import numpy as np
import random

class QUBO:

    def __init__(self, fname):
        self.fname = fname
        self.readQUBO()

    def readQUBO(self):

        with open(self.fname, 'rt') as file:
            problem = json.load(file)
            self.pname = problem.get('problem')
            self.nbit = problem.get('nbit')
            self.qubo = problem.get('qubo')
            self.base = problem.get('base')
            self.vertice = []
            self.nvertice = []

        self.qubomatrix = np.zeros((self.nbit, self.nbit), int)
        if self.base == 0:
            for i in range(len(self.qubo)):
                x, y, w = self.qubo[i][0], self.qubo[i][1], self.qubo[i][2]
                self.qubomatrix[x, y] = w

        else:
            for i in range(len(self.qubo)):
                x, y, w = self.qubo[i][0], self.qubo[i][1], self.qubo[i][2]
                self.qubomatrix[x - 1, y - 1] = w

        self.qubomatrix += np.triu(self.qubomatrix, 1).T
        self.diag = np.diag(self.qubomatrix)
        for i in range(self.nbit):
            Idx = np.nonzero(self.qubomatrix[i])[0]
            self.vertice.append(Idx[Idx != i])
            self.nvertice.append(Idx[Idx != i].size)

        self.printQUBO()

    def printQUBO(self):
        print('problem: ', self.pname)
        print('nbit = ', self.nbit)
        print('qubo = ', self.qubomatrix)
        print('base = ', self.base)


class Queue:

    def __init__(self, queue_len):
        self.queue_len = queue_len
        self.queue = []

    def enqueue(self, new_elem):
        if not self.is_full():
            self.queue.append(new_elem)
        else:
            self.dequeue()
            self.queue.append(new_elem)

    def dequeue(self):
        return self.queue.pop(0)

    def is_empty(self):
        return not self.queue

    def is_full(self):
        if len(self.queue) == self.queue_len:
            return True
        else:
            return False


class Solution:

    def __init__(self, problem):
        self.problem = problem
        self.energy = 0
        self.op_energy = 0
        self.bit = np.zeros(self.problem.nbit, int)
        self.op_bit = tuple(np.zeros(self.problem.nbit, int))
        self.dE = self.problem.diag.astype('int32')
        self.queue_len = 20

    def flipbit(self, flipIdx):
        dir = 2 * self.bit[flipIdx] - 1
        self.energy += self.dE[flipIdx]
        self.bit[flipIdx] = 1 - self.bit[flipIdx]
        self.dE[flipIdx] = -self.dE[flipIdx]
        for i in range(self.problem.nvertice[flipIdx]):
            j = self.problem.vertice[flipIdx][i]
            self.dE[j] += self.problem.qubomatrix[flipIdx][j] * dir * (2 * self.bit[j] - 1)

    def greedy(self):
        minIdx = np.argmin(self.dE)
        if self.dE[minIdx] < 0:
            self.flipbit(minIdx)
            return True
        else:
            return False

    def greedy_loop(self):
        while self.greedy():
            pass
        if self.energy < self.op_energy:
            self.op_energy = self.energy
            self.op_bit = tuple(self.bit)

    def SA(self, flipIdx, T):
        if self.dE[flipIdx] < 0:
            res = 1
        else:
            num = random.random()
            eps = np.exp(-self.dE[flipIdx] / T)
            if (eps > num and eps < 1):
                res = 2
            else:
                res = 0
        return res

    def get_newIdx(self, oldIdx):
        while True:
            newIdx = random.randint(0, (self.problem.nbit - 1))
            if newIdx != oldIdx:
                break
        return newIdx

    def solution_pool(self, energy, bit):
        pass

    def SA_loop(self, T, ET, dt, times, limit):
        elem_queue = Queue(self.queue_len)
        oldIdx = 0
        t = T
        while t > ET:
            unc = 0
            for i in range(times):
                newIdx = self.get_newIdx(oldIdx)
                res = self.SA(newIdx, t)
                if res == 1:
                    self.flipbit(newIdx)
                    if self.energy < self.op_energy:
                        self.op_energy = self.energy
                        self.op_bit = tuple(self.bit)
                    elem_queue.enqueue(newIdx)
                    oldIdx = newIdx
                    unc = 0
                elif res == 2:
                    if elem_queue.queue.count(newIdx) == 0:
                        self.flipbit(newIdx)
                        if self.energy < self.op_energy:
                            self.op_energy = self.energy
                            self.op_bit = tuple(self.bit)
                        elem_queue.enqueue(newIdx)
                        oldIdx = newIdx
                        unc = 0
                elif res == 0:
                    unc += 1
                    if unc > limit:
                        break
            t = t * dt

    def Max_Min_SA(self, iter_times, iter_tem):
        min_dE = min(self.dE)
        max_dE = max(self.dE)
        temIdx = []
        p = random.random()
        th = min_dE + (max_dE - min_dE) * (iter_times - iter_tem) / iter_times * p
        for i in range(self.problem.nbit):
            if self.dE[i] <= th:
                temIdx.append(i)
        ran_in_temIdx = random.randint(0, len(temIdx) - 1)
        return temIdx[ran_in_temIdx]

    def Max_Min_SA_loop(self, T, ET, dt, times, limit):
        elem_queue = Queue(self.queue_len)
        t = T
        while t > ET:
            unc = 0
            for i in range(times):
                newIdx = self.Max_Min_SA(times, i)
                res = self.SA(newIdx, t)
                if res == 1:
                    self.flipbit(newIdx)
                    if self.energy < self.op_energy:
                        self.op_energy = self.energy
                        self.op_bit = tuple(self.bit)
                    elem_queue.enqueue(newIdx)
                    unc = 0
                elif res == 2:
                    if elem_queue.queue.count(newIdx) == 0:
                        self.flipbit(newIdx)
                        if self.energy < self.op_energy:
                            self.op_energy = self.energy
                            self.op_bit = tuple(self.bit)
                        elem_queue.enqueue(newIdx)
                        unc = 0
                elif res == 0:
                    unc += 1
                    if unc > limit:
                        break
            t = t * dt

    def Pos_Min_Min_SA(self, dB):
        ri = []
        #for i in range(self.problem.nbit):
            #R.append(random.randint(1,10))
        temIdx = []
        dra_dE = list(map(lambda x:-dB if x==0 else x, self.dE))

        for i in range(self.problem.nbit):
            R = random.randint(0, 10)
            ri.append(np.mod(R, dB))
        en_dE = [dB * x for x in dra_dE]
        new_dE = [x + y for x, y in zip(en_dE, ri)]
        pos_dE = [x for x in new_dE if x > 0]
        if pos_dE == []:
            pos_min_dE = 0
        else:
            pos_min_dE = min(pos_dE)
        for i in range(self.problem.nbit):
            if new_dE[i] <= pos_min_dE:
                temIdx.append(i)
        ran_in_temIdx = random.randint(0, len(temIdx) - 1)
        return temIdx[ran_in_temIdx]


    def Pos_Min_Min_SA_loop(self, T, ET, dt, times, limit,dB):
        elem_queue = Queue(self.queue_len)
        t = T
        while t > ET:
            unc = 0
            for i in range(times):
                newIdx = self.Pos_Min_Min_SA(dB)
                res = self.SA(newIdx, t)
                if res == 1:
                    self.flipbit(newIdx)
                    if self.energy < self.op_energy:
                        self.op_energy = self.energy
                        self.op_bit = tuple(self.bit)
                    elem_queue.enqueue(newIdx)
                    unc = 0
                elif res == 2:
                    if elem_queue.queue.count(newIdx) == 0:
                        self.flipbit(newIdx)
                        if self.energy < self.op_energy:
                            self.op_energy = self.energy
                            self.op_bit = tuple(self.bit)
                        elem_queue.enqueue(newIdx)
                        unc = 0
                elif res == 0:
                    unc += 1
                    if unc > limit:
                        break
            t = t * dt

    def Cyclic_Min(self, start, end):
        if end > start:
            minIdx = start + np.argmin(self.dE[start:end])
            if minIdx > self.problem.nbit:
                minIdx -= self.problem.nbit
        else:
            vector = np.concatenate((self.dE[start:self.problem.nbit], self.dE[0:end]))
            minIdx = start + np.argmin(vector)
            if minIdx > self.problem.nbit:
                minIdx -= self.problem.nbit
        return minIdx

    def Cyclic_Min_loop(self, initial_range, flip_times):
        start = 0
        flip_range = initial_range
        while flip_range != self.problem.nbit:
            end = start + flip_range
            for i in range(flip_times):
                newIdx = self.Cyclic_Min(start, end)
                #print(f'start={start},end={end},i={i}')
                #print(self.dE[newIdx])
                self.flipbit(newIdx)
                if self.energy < self.op_energy:
                    self.op_energy = self.energy
                    self.op_bit = tuple(self.bit)
                start += flip_range
                if start == self.problem.nbit:
                    start = 0
                elif start > self.problem.nbit:
                    start -= self.problem.nbit
                end = start + flip_range
                if end > self.problem.nbit:
                    end -= self.problem.nbit
            #start = end
            flip_range = 2*flip_range
        self.greedy_loop()

def main():
    fname = 'C:\\Users\\84918\\Desktop\\QUBO_test\\4096.json'  #debugしやすいため、しばらくアドレスを固定にする
    size = 8
    problem = QUBO(fname)
    result = Solution(problem)
    #result.greedy_loop()
    #result.SA_loop(1000, 1e-5, 0.95, 2000, 1800)
    #result.Pos_Min_Min_SA_loop(1000, 1e-5, 0.95, 1000, 800, 1000)
    #result.Max_Min_SA_loop(1000, 1e-5, 0.95, 1000, 800)
    result.Cyclic_Min_loop(16,10000)
    bit_array = np.zeros((size, size), int)
    k = 0
    #for i in range(size):
        #for j in range(size):
            #bit_array[i][j] = result.op_bit[k]
           # k += 1

    print('bit=', result.op_bit)
    print('energy=', result.op_energy)


if __name__ == "__main__":
    main()
