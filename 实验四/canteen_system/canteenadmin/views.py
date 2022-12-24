from django.shortcuts import render
from . import models
from django.shortcuts import redirect
from .forms import AdminForm


def canteen_list(request):
    context = {
        'canteen_list': models.Canteen.objects.all(),
    }
    return render(request, 'show/canteen.html', context)


def news_list(request):
    context = {
        'news_list': models.News.objects.all(),
    }
    return render(request, 'show/news.html', context)


def login(request):
    login_form = AdminForm()
    if request.session.get('is_login', None):  # 不允许重复登录
        return redirect('/user/index/')
    if request.method == 'POST':
        login_form = AdminForm(request.POST)
        message = '请检查填写的内容！'
        if login_form.is_valid():
            admin_name = login_form.cleaned_data.get('admin_name')
            password = login_form.cleaned_data.get('password')

            try:
                admin = models.Admininfo.objects.get(admin_name=admin_name)
            except:
                message = '用户不存在！'
                return render(request, 'canteenadmin/login.html', locals())

            if admin.admin_password == password:
                request.session['is_login'] = True
                request.session['admin_id'] = admin.admin_id
                request.session['admin_name'] = admin.admin_name
                return redirect('/admin/')
            else:
                message = '密码不正确！'
                return render(request, 'canteenadmin/login.html', locals())
        else:
            return render(request, 'canteenadmin/login.html', locals())

    login_form = AdminForm()
    return render(request, 'canteenadmin/login.html', locals())
