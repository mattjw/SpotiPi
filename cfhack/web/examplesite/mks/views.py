from django.shortcuts import render_to_response, redirect, get_object_or_404
from django.template import RequestContext

from mks.models import User
from mks.forms import UserForm

def users(request):
    latest_user_list = User.objects.all().order_by('name')
    print len(latest_user_list)
    if len(latest_user_list) < 1:
        return redirect(user_add)
    else:
        return render_to_response('mks/user.html',
		{'latest_user_list': latest_user_list,})

def user_add(request):
    # sticks in a POST or renders empty form
    form = UserForm(request.POST or None)
    if form.is_valid():
        cmodel = form.save()
        #This is where you might chooose to do stuff.
        cmodel.save()
        return redirect(users)

    return render_to_response('mks/user_add.html',
                              {'user_form': form},
                              context_instance=RequestContext(request))
                              
def user_edit(request, user_id):
    user = get_object_or_404(User, pk=user_id)
    form = UserForm(request.POST or None, instance=user)
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

    return redirect(users)                          
