from django.db import models
from django import forms

class User(models.Model):
    name = models.CharField(max_length=20)
    password = models.CharField(max_length=10)
    def __unicode__(self):
        return self.name
