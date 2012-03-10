from piston.handler import BaseHandler
import subprocess

class SpotHandler(BaseHandler):
    def read(self, request, expression):
        if expression == 'isEnabled':
            return "y"
        else:
            return "n"
   
    # def create(self, request):
    #     if expression == 'enable':
    #         process = subprocess.call('ls -a')
    #         return process
    #     elif expression == 'next':
    #         return expression
    #     elif expression == 'disable':
    #         return "disabled"

    def update(self, request, expression):
        if expression == 'enable':
            # process = subprocess.call('ls -a')
            # return request.data
            return expression
        elif expression == 'next':
            return expression
        elif expression == 'disable':
            return expression
        else:
            return 'unrecognised call, ignored'