from piston.handler import BaseHandler
from mks.models import User, Playlists
import subprocess

class SpotHandler(BaseHandler):
    def read(self, request, expression):
        if expression == 'isEnabled':
            return "y"
        else:
            return "n"

# all PUT requests handler here, anything other than the defined keywords are ignored
    def update(self, request, expression):
        if expression == 'enable':
            i = request.PUT.get('ID')
            print i
            user = User.objects.filter(id=i)[0]
            user.enabled = True
            user.save()
            return user.enabled
        elif expression == 'next':
            return_code = subprocess.Popen("echo hello string", shell=True)
            return 'next'
        elif expression == 'disable':
            i = request.PUT.get('ID')
            print i
            user = User.objects.filter(id=i)[0]
            user.enabled = False
            user.save()
            return user.enabled
        elif expression == 'set_playlist':
            user_id = request.PUT.get('ID')
            pl_id = request.PUT.get('PL_ID')
            try:
                user = User.objects.filter(id=user_id)[0]
                print user.name
                user.default_pl = pl_id
                print user.default_pl
                user.save()
                return user.default_pl
            except e:
                return e
        else:
            return 'unrecognised call, ignored'