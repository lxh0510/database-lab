from django.contrib import admin

from .models import Business, Meal,Orderinfo,OrderMeal,OrderList


class BusinessManager(admin.ModelAdmin):
    list_per_page = 10
    list_display = ['business_id', 'business_phone', 'storename', 'business_status']


class OrderManager(admin.ModelAdmin):
    list_per_page = 10
    list_display = ['order_id', 'user', 'new_name', 'total_price',  'order_time']


admin.site.register(Business, BusinessManager)
admin.site.register(Meal)
admin.site.register(Orderinfo)
admin.site.register(OrderMeal)
admin.site.register(OrderList)



# Register your models here.
