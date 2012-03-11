from django.forms import ModelForm, ModelChoiceField
from mks.models import User, Playlists
from django import forms

# not used
class UserForm(ModelForm):
    name  = forms.CharField(label="Username" )
    password = forms.CharField( widget=forms.PasswordInput, label="Password")
    default_pl = ModelChoiceField(label="Default Playlist", queryset=Playlists.objects.values_list('name'), required=False)
    class Meta:
        model = User

# cannot find a way to modify the placeholder on two text input fields, django did not like the repetition
class PartialUserForm(ModelForm):
    name  = forms.CharField(widget=forms.TextInput(attrs={'placeholder': 'Username'}), label="" )
    password = forms.CharField( widget=forms.PasswordInput, label="")
    class Meta:
        model = User
        exclude = ('default_pl', 'enabled')
