import numpy as np
import sys

class Layer:

    def __init__(self, list=[], flow=None):
        self.list = list
        self.flow = flow
        self.cnt = 0
        self.limit = sys.maxsize

        if len(list) > 0:
            self.limit = len(list)

        self.filter = lambda x: True
        self.function = lambda x: x

    def __iter__(self):
        return self

    def __next__(self):
        return self.next()

    def next(self):
        if self.cnt >= self.limit:
            raise StopIteration()
        if len(self.list) > 0:
            return_elem = self.list[self.cnt]
        if self.flow is not None:
            return_elem = self.flow.next()
        self.cnt += 1
        if self.filter(return_elem):
            return self.function(return_elem)
        else:
            return self.next()

    def take(self, lim_num):
        self.limit = min(self.limit, lim_num)
        return self

    def select(self, fun):
        new_layer = Layer(flow=self)
        new_layer.function = fun
        return new_layer

    def where(self, filt):
        new_layer = Layer(flow=self)
        new_layer.filter = filt
        return new_layer

    def toList(self):
        return_list = []
        for elem in self:
            return_list.append(elem)
        return return_list

    def groupBy(self, key_fun):
        group_dict = {}
        new_list = self.toList()
        for elem in new_list:
            elem_key = key_fun(elem)
            group_dict.setdefault(elem_key, []).append(elem)
        return Layer([(elem_key, group_dict[elem_key]) for elem_key in group_dict])

    def orderBy(self, comp):
        return Layer(sorted(self.toList(), key=comp))

    def flatten(self):
        list_for_flat = self.toList()
        flatten_list = []
        for elem in list_for_flat:
            flatten_list += elem.toList()
        return Layer(flatten_list)


class FibFlow:
    def __init__(self):
        self.first = 1
        self.second = 1

    def next(self):
        self.first, self.second = self.second, self.first + self.second
        return self.second

class TextFlow:
    def __init__(self, file):
        self.file = file

    def next(self):
        string = self.file.readline()
        if string == "":
            raise StopIteration()
        else:
            return string


text_flow = TextFlow(open('input.txt'))
text_object = Layer(flow=text_flow)


print(text_object
      .select(lambda x: Layer(list=x.split(' ')))
      .flatten()
      .groupBy(lambda x: x.lower())
      .select(lambda x: (x[0], len(x[1])))
      .orderBy(lambda x: -x[1])
      .toList())

fib_flow = FibFlow()
object = Layer(flow=fib_flow)
print(object.where(lambda x: x % 3 == 0).select(lambda x: x**2 if x % 2 == 0 else x).take(5).toList())