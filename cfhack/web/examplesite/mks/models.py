from django.db import models
from django import forms

class Playlists(models.Model):
	pl_id = models.IntegerField(primary_key=True)
	name = models.CharField(max_length=100)
	no_of_songs = models.IntegerField()
	class Meta:
		db_table='spotify_playlists'

class User(models.Model):
    name = models.CharField(max_length=20)
    password = models.CharField(max_length=10)
    default_pl = models.ForeignKey(Playlists, null=True)
    enabled = models.BooleanField(default=False)
    def __unicode__(self):
        return self.name
    class Meta:
        db_table='users'
