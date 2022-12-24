# This is an auto-generated Django model module.
# You'll have to do the following manually to clean this up:
#   * Rearrange models' order
#   * Make sure each model has one field with primary_key=True
#   * Make sure each ForeignKey has `on_delete` set to the desired behavior.
#   * Remove `managed = False` lines if you wish to allow Django to create, modify, and delete the table
# Feel free to rename the models, but don't rename db_table values or field names.
from django.db import models
from django.urls import reverse


class Business(models.Model):
    business_id = models.AutoField(primary_key=True, verbose_name='商家编号')
    canteen = models.ForeignKey('canteenadmin.Canteen', models.CASCADE, blank=True, null=True, verbose_name='食堂编号')
    business_name = models.CharField(max_length=10, verbose_name='商家用户名')
    business_password = models.CharField(max_length=10, verbose_name='商家密码')
    business_phone = models.DecimalField(max_digits=11, decimal_places=0, verbose_name='商家电话')
    storename = models.CharField(max_length=10, verbose_name='店名')
    business_status = models.IntegerField(choices=[(1, '营业中'), (0, '休息中')], verbose_name='营业状态')
    business_graph = models.ImageField(verbose_name='菜品图片')

    class Meta:
        ordering = ['business_id']
        db_table = 'business'
        verbose_name = "商家信息"
        verbose_name_plural = verbose_name

    def __str__(self):
        return self.storename


class Meal(models.Model):
    meal_id = models.AutoField(primary_key=True, verbose_name='菜品编号')
    business = models.ForeignKey(Business, models.CASCADE, blank=True, null=True, verbose_name='商家')
    category = models.ForeignKey('canteenadmin.Category', models.CASCADE, blank=True, null=True, verbose_name='种类')
    meal_name = models.CharField(max_length=10, verbose_name='菜品名称')
    meal_description = models.TextField(verbose_name='菜品描述')
    meal_left = models.IntegerField(verbose_name='菜品库存')
    meal_graph = models.ImageField(verbose_name='菜品图片')
    meal_price = models.IntegerField(verbose_name='菜品价格')

    class Meta:
        ordering = ['meal_id']
        db_table = 'meal'
        verbose_name = "菜品信息"
        verbose_name_plural = verbose_name

    def __str__(self):
        return self.meal_name

    def put_list_url(self):
        return reverse("business:put_list", kwargs={'meal_id': self.meal_id})


class Orderinfo(models.Model):
    order_id = models.AutoField(primary_key=True, verbose_name='订单编号')
    user = models.ForeignKey('user.Userinfo', models.CASCADE, blank=True, null=True, verbose_name='用户')
    new_name = models.CharField(max_length=10, blank=True, null=True, verbose_name='备注名')
    total_price = models.FloatField(verbose_name='总价')
    address = models.TextField(verbose_name='地址')
    order_note = models.TextField(verbose_name='备注')
    arrive_time = models.CharField(max_length=10, verbose_name='预定送达时间')
    order_time = models.CharField(max_length=50, verbose_name='下单时间')

    class Meta:
        ordering = ['-order_time']
        db_table = 'orderinfo'
        verbose_name = "订单信息"
        verbose_name_plural = verbose_name

    def __str__(self):
        return str(self.order_id)


class OrderMeal(models.Model):
    meal = models.ForeignKey(Meal, None, verbose_name='菜品')
    user = models.ForeignKey('user.Userinfo', None, verbose_name='用户')
    meal_number = models.IntegerField(verbose_name='菜品数量')

    class Meta:
        ordering = ['user_id', 'meal_id']
        db_table = 'order_meal'
        verbose_name = "用户购物车信息"
        verbose_name_plural = verbose_name
        unique_together = (('meal', 'user'),)

    def __str__(self):
        return '%s: %s %d' % (str(self.user), str(self.meal), self.meal_number)


class OrderList(models.Model):
    meal = models.ForeignKey(Meal, None, verbose_name='菜品')
    order = models.ForeignKey(Orderinfo, None, verbose_name='订单')
    meal_number = models.IntegerField(verbose_name='菜品数量')
    order_status = models.IntegerField(choices=[(0, '已下单'), (1, '已接单')], verbose_name='订单状态')

    class Meta:
        ordering = ['order_id', 'meal_id']
        db_table = 'order_list'
        verbose_name = "订单中菜品信息"
        verbose_name_plural = verbose_name
        unique_together = (('meal', 'order'),)

    def __str__(self):
        return '%s: %s %d' % (str(self.order), str(self.meal), self.meal_number)
