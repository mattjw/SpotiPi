from django.db import models
from django import forms

class Playlists(models.Model):
	pl_id = models.CharField(primary_key=True, max_length=10)
	name = models.CharField(max_length=100)

class User(models.Model):
    name = models.CharField(max_length=20)
    password = models.CharField(max_length=10)
    default_pl = models.ForeignKey(Playlists, null=True)
    def __unicode__(self):
        return self.name
