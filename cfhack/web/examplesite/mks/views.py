from django.shortcuts import render_to_response, redirect, get_object_or_404
from django.template import RequestContext
import os

from mks.models import User, Playlists
from mks.forms import UserForm, PartialUserForm

def users(request):
    playlists_list = Playlists.objects.all().order_by('pl_id')
    try:
        user = User.objects.all().order_by('name')[0]
        return render_to_response('mks/user.html',
        {'user': user, 'playlists_list': playlists_list})
    except:
        form = PartialUserForm(request.POST or None)
        # return user_add(request)
        return render_to_response('mks/user.html',
        {'user': None, 'playlists_list': playlists_list, 'user_form': form})
        

def user_add(request):
    # sticks in a POST or renders empty form
    form = PartialUserForm(request.POST or None)
    if form.is_valid():
        cmodel = form.save()
        #This is where you might chooose to do stuff.
        cmodel.save()
        os.system("ls -a");
        return redirect(users)

    return render_to_response('mks/user_add.html',
                              {'user_form': form},
                              context_instance=RequestContext(request))
                              
def user_edit(request, user_id):
    user = get_object_or_404(User, pk=user_id)
    form = PartialUserForm(request.POST or None, instance=user)
    if form.is_valid():
        user = form.save()
        #this is where you might choose to do stuff.
        #user.name = 'test'
        user.save()
        return redirect(users)

    return render_to_response('mks/user_edit.html',
                              {'user_form': form,
                               'user_id': user_id},
                              context_instance=RequestContext(request))
                              
def user_delete(request, user_id):
    c = User.objects.get(pk=user_id).delete()
    Playlists.objects.all().delete()

    return redirect(users)                          
