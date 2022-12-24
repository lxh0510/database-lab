"""canteen_system URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/2.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path
from django.urls import re_path
from django.urls import include
from django.views.static import serve
from .settings import MEDIA_ROOT
from user import views as user_views
from canteenadmin import views as canteen_views
from django.urls import include, path

urlpatterns = [
    re_path(r'media/(?P<path>.*)$',serve,{'document_root':MEDIA_ROOT}),
    path('admin/', admin.site.urls),
    path('user/login/', user_views.login),
    path('user/index/', user_views.index),
    path('user/register/', user_views.register),
    path('user/logout/', user_views.logout),
    path('business/', include('business.urls')),
    path('canteenadmin/login/', canteen_views.login),
    path('canteen/',canteen_views.canteen_list),
    path('news/', canteen_views.news_list),
    path('index/', user_views.index),
]
