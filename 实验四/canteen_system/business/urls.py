from django.urls import path
from .views import login, meal_list, put_list, show_list, put_order, show_order, business_list, logout, index, put_news, \
    order_1_list, receive_order, order_2_list

app_name = 'business'
urlpatterns = [
    path('login/', login),
    path('logout/', logout),
    path('index/', index),
    path('dish/', meal_list),
    path('put_list/<slug:meal_id>', put_list, name='put_list'),
    path('show_list/', show_list),
    path('put_order/', put_order),
    path('my_order/', show_order),
    path('put_news/', put_news),
    path('business_list/', business_list),
    path('order_1_list/', order_1_list),
    path('receive_order/', receive_order),
    path('order_2_list/', order_2_list)
]
