from django.forms import ModelForm
from mks.models import User
from django import forms

class UserForm(ModelForm):
    name  = forms.CharField(label="Your Username" )
    password = forms.CharField( widget=forms.PasswordInput, label="Your Password" )
    class Meta:
        model = User
