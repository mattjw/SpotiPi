from django.forms import ModelForm, ModelChoiceField
from mks.models import User, Playlists
from django import forms

class UserForm(ModelForm):
    name  = forms.CharField(label="Your Username" )
    password = forms.CharField( widget=forms.PasswordInput, label="Your Password" )
    default_pl = ModelChoiceField(label="Default Playlist", queryset=Playlists.objects.values_list('name'), required=False)
    class Meta:
        model = User

class PartialUserForm(ModelForm):
    name  = forms.CharField(label="Your Username" )
    password = forms.CharField( widget=forms.PasswordInput, label="Your Password" )
    class Meta:
        model = User
        exclude = ('default_pl', 'enabled')
