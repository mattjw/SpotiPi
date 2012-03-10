from django.conf.urls.defaults import patterns, include, url

urlpatterns = patterns('',
    url(r'^$', 'mks.views.users'),  
    url(r'^user_add/$', 'mks.views.user_add'),
    url(r'^user_edit/(?P<user_id>\d+)/$',
		'mks.views.user_edit'),
    url(r'^user_delete/(?P<user_id>\d+)/$',
		'mks.views.user_delete'),
)
