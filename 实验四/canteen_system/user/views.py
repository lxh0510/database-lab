# 视图函数,返回index.html页面
from django.http import HttpResponse
from django.shortcuts import render
from django.shortcuts import redirect
from . import models
from .forms import UserForm,RegisterForm


def login(request):
    login_form = UserForm()
    if request.session.get('is_login', None):  # 不允许重复登录
        return redirect('/user/index/')
    if request.method == 'POST':
        login_form = UserForm(request.POST)
        message = '请检查填写的内容！'
        if login_form.is_valid():
            username = login_form.cleaned_data.get('username')
            password = login_form.cleaned_data.get('password')

            try:
                user = models.Userinfo.objects.get(user_name=username)
            except:
                message = '用户不存在！'
                return render(request, 'user/login.html', locals())

            if user.user_password == password:
                request.session['is_login'] = True
                request.session['user_id'] = user.user_id
                request.session['user_name'] = user.user_name
                return redirect('/index/')
            else:
                message = '密码不正确！'
                return render(request, 'user/login.html', locals())
        else:
            return render(request, 'user/login.html', locals())

    login_form = UserForm()
    return render(request, 'user/login.html', locals())


def register(request):
    register_form = RegisterForm()
    if request.method == 'POST':
        register_form = RegisterForm(request.POST)
        message = '请检查填写的内容！'
        if register_form.is_valid():
            username = register_form.cleaned_data.get('username')
            password1 = register_form.cleaned_data.get('password1')
            password2 = register_form.cleaned_data.get('password2')
            phone = register_form.cleaned_data.get('phone')

            if password1 != password2:
                message = '两次输入的密码不同'
                return render(request, 'user/register.html', locals())
            else:
                user = models.Userinfo.objects.filter(user_name=username)
                if user:
                    message = '用户已存在！'
                    return render(request, 'user/register.html', locals())
                else:
                    new_user = models.Userinfo.objects.create(user_name=username, user_password=password1,
                                                              user_phone=phone)
                    new_user.save()
                    login_form = UserForm()
                    message = '注册成功！'
                    return render(request, 'user/login.html', locals())
    else:
        return render(request, 'user/register.html', locals())
    register_form = RegisterForm()
    return render(request, 'user/register.html')


def logout(request):
    if not request.session.get('is_login', None):
        # 如果本来就未登录，也就没有登出一说
        return render(request, 'index.html')
    request.session.flush()
    return render(request, 'index.html')


def index(request):
    return render(request, 'index.html')
