from django.conf.urls.defaults import *
from piston.resource import Resource
from api.handlers import *

# pistons recommended method for dealing with CSRF
class CsrfExemptResource(Resource):
    def __init__(self, handler, authentication = None):
        super(CsrfExemptResource, self).__init__(handler, authentication)
        self.csrf_exempt = getattr(self.handler, 'csrf_exempt', True)

spot_resource  = CsrfExemptResource(SpotHandler)
# anything called after api is handled by SpotHandler, through the spot_resource
urlpatterns = patterns('',
    (r'^(?P<expression>.*)$', spot_resource),
)
