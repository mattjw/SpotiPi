from django.contrib import admin

from mks.models import User

class UserInline(admin.StackedInline):
    model = User
    extra = 1

class UserAdmin(admin.ModelAdmin):
	inlines = [UserInline]

admin.site.register(User, UserAdmin)




