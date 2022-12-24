from django.shortcuts import render
from django.shortcuts import redirect
from django.shortcuts import get_object_or_404
from django.contrib import messages
from django.core.exceptions import ObjectDoesNotExist
from user.models import Userinfo
from canteenadmin.models import News
from . import models
from .forms import BusinessForm, OrderForm, NewsForm
from django.utils import timezone
import pytz


def login(request):
    login_form = BusinessForm()
    if request.session.get('is_login', None):  # 不允许重复登录
        return redirect('/business/index/')
    if request.method == 'POST':
        login_form = BusinessForm(request.POST)
        message = '请检查填写的内容！'
        if login_form.is_valid():
            business_name = login_form.cleaned_data.get('business_name')
            password = login_form.cleaned_data.get('password')

            try:
                business = models.Business.objects.get(business_name=business_name)
            except:
                message = '用户不存在！'
                return render(request, 'business/login.html', locals())

            if business.business_password == password:
                request.session['is_login'] = True
                request.session['business_id'] = business.business_id
                request.session['business_name'] = business.business_name
                return redirect('/business/index/')
            else:
                message = '密码不正确！'
                return render(request, 'business/login.html', locals())
        else:
            return render(request, 'business/login.html', locals())

    login_form = BusinessForm()
    return render(request, 'business/login.html', locals())


def logout(request):
    if not request.session.get('is_login', None):
        # 如果本来就未登录，也就没有登出一说
        return render(request, 'business_index.html')
    request.session.flush()
    return render(request, 'business_index.html')


def index(request):
    return render(request, 'business_index.html')


def meal_list(request):
    context = {
        'meal_list': models.Meal.objects.all(),
    }
    return render(request, 'show/meal.html', context)


def put_list(request, meal_id):
    meal = get_object_or_404(models.Meal, meal_id=meal_id)
    if not request.session.get('is_login', None):
        messages.warning(request, "请先登录顾客账户~")
        return redirect('/user/login/')
    user_id = request.session['user_id']
    try:
        user = Userinfo.objects.filter(user_id=user_id).first()
        order_meal = models.OrderMeal.objects.filter(meal=meal, user=user).first()
        if order_meal:
            order_meal.meal_number = order_meal.meal_number + 1
            order_meal.save()
        else:
            new_order_meal = models.OrderMeal.objects.create(meal=meal, user=user, meal_number=1)
            new_order_meal.save()
        messages.success(request, '已加入购物车!')
        return redirect('/business/dish/')
    except ObjectDoesNotExist:
        messages.warning(request, '加入失败!')
        return redirect("/business/dish/")


def show_list(request):
    if not request.session.get('is_login', None):
        messages.warning(request, "请先登录顾客账户~")
        return redirect('/user/login/')
    user_id = request.session['user_id']
    user = Userinfo.objects.filter(user_id=user_id).first()
    context = {
        'shop_list': models.OrderMeal.objects.filter(user=user),
    }
    return render(request, 'user/shoplist.html', context)


def put_order(request):
    order_form = OrderForm()
    if request.method == 'POST':
        order_form = OrderForm(request.POST)
        message = '请检查填写的内容！'
        if order_form.is_valid():
            new_name = order_form.cleaned_data.get('new_name')
            address = order_form.cleaned_data.get('address')
            arrive_time = order_form.cleaned_data.get('arrive_time')
            note = order_form.cleaned_data.get('note')

            user_id = request.session['user_id']
            user = Userinfo.objects.filter(user_id=user_id).first()
            tz = pytz.timezone('Asia/Shanghai')
            order_time = timezone.now().astimezone(tz=tz).strftime("%Y.%m.%d %H:%M:%S")
            order_list = models.OrderMeal.objects.filter(user=user)
            total_price = 0
            for order in order_list:
                total_price += order.meal.meal_price * order.meal_number
            new_order = models.Orderinfo.objects.create(user=user, new_name=new_name, total_price=total_price,
                                                        address=address, order_note=note,
                                                        arrive_time=arrive_time, order_time=order_time)
            new_order.save()
            for order in order_list:
                new_list = models.OrderList.objects.create(meal=order.meal, order=new_order,
                                                           meal_number=order.meal_number, order_status=0)
                new_list.save()
            models.OrderMeal.objects.filter(user=user).delete()
            message = '填写成功'
            return render(request, '/business/my_order/', locals())
        else:
            order_form = OrderForm()
            return render(request, 'user/put_order.html', locals())
    else:
        order_form = OrderForm()
        return render(request, 'user/put_order.html', locals())


def show_order(request):
    if not request.session.get('is_login', None):
        messages.warning(request, "请先登录顾客账户~")
        return redirect('/user/login/')
    user_id = request.session['user_id']
    user = Userinfo.objects.filter(user_id=user_id).first()
    context = {
        'order_list': models.Orderinfo.objects.filter(user=user),
    }
    return render(request, 'user/my_order.html', context)


def business_list(request):
    context = {
        'business_list': models.Business.objects.all(),
    }
    return render(request, 'show/business.html', context)


def put_news(request):
    order_form = OrderForm()
    if request.method == 'POST':
        news_form = NewsForm(request.POST)
        message = '请检查填写的内容！'
        if news_form.is_valid():
            news_title = news_form.cleaned_data.get('news_title')
            news_content = news_form.cleaned_data.get('news_content')

            business_id = request.session['business_id']
            business = models.Business.objects.filter(business_id=business_id).first()
            news = News.objects.create(business=business, news_title=news_title, news_content=news_content)
            news.save()
            message = '填写成功'
            return redirect('/business/index/')
        else:
            news_form = NewsForm()
            return render(request, 'business/put_order.html', locals())


def order_1_list(request):
    business_id = request.session['business_id']
    context = {
        'order_list': models.OrderList.objects.filter(order_status=0),
        'business': models.Business.objects.filter(business_id=business_id).first(),
        'order_info': models.Orderinfo.objects.all()
    }
    return render(request, 'business/get_order.html', context)


def receive_order(request):
    business_id = request.session['business_id']
    business = models.Business.objects.filter(business_id=business_id).first()
    order_list = models.OrderList.objects.filter(order_status=0)
    for order in order_list:
        if order.meal.business == business:
            order.order_status = 1
            order.save()
    return redirect('/business/order_2_list/')


def order_2_list(request):
    business_id = request.session['business_id']
    context = {
        'order_list': models.OrderList.objects.filter(order_status=1),
        'business': models.Business.objects.filter(business_id=business_id).first(),
        'order_info': models.Orderinfo.objects.all()
    }
    return render(request, 'business/show_order.html', context)
