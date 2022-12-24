# This is an auto-generated Django model module.
# You'll have to do the following manually to clean this up:
#   * Rearrange models' order
#   * Make sure each model has one field with primary_key=True
#   * Make sure each ForeignKey has `on_delete` set to the desired behavior.
#   * Remove `managed = False` lines if you wish to allow Django to create, modify, and delete the table
# Feel free to rename the models, but don't rename db_table values or field names.
from django.db import models


class Admininfo(models.Model):
    admin_id = models.AutoField(primary_key=True, verbose_name='管理员编号')
    admin_name = models.CharField(max_length=10, verbose_name='管理员账号名')
    admin_password = models.CharField(max_length=10, verbose_name='管理员密码')
    admin_phone = models.DecimalField(max_digits=11, decimal_places=0, verbose_name='管理员电话')

    class Meta:
        ordering = ['admin_id']
        db_table = 'admininfo'
        verbose_name = "管理员信息"
        verbose_name_plural = verbose_name

    def __str__(self):
        return self.admin_name


class Canteen(models.Model):
    canteen_id = models.AutoField(primary_key=True, verbose_name='食堂编号')
    canteen_name = models.CharField(max_length=5, verbose_name='食堂名称')
    open_time = models.TimeField(verbose_name='开业时间')
    end_time = models.TimeField(verbose_name='结业时间')
    canteen_graph = models.ImageField(verbose_name='食堂图片')

    class Meta:
        ordering = ['canteen_id']
        db_table = 'canteen'
        verbose_name = "食堂信息"
        verbose_name_plural = verbose_name

    def __str__(self):
        return self.canteen_name


class Category(models.Model):
    category_id = models.AutoField(primary_key=True, verbose_name='种类编号')
    category_name = models.CharField(max_length=10, verbose_name='种类名')

    class Meta:
        ordering = ['category_id']
        db_table = 'category'
        verbose_name = "种类信息"
        verbose_name_plural = verbose_name

    def __str__(self):
        return self.category_name


class News(models.Model):
    news_id = models.AutoField(primary_key=True, verbose_name='咨询编号')
    business = models.ForeignKey('business.Business', models.CASCADE, blank=True, null=True, verbose_name='商家')
    news_title = models.TextField(verbose_name='资讯标题')
    news_content = models.TextField(verbose_name='资讯内容')

    class Meta:
        ordering = ['news_id']
        db_table = 'news'
        verbose_name = "美食资讯"
        verbose_name_plural = verbose_name

    def __str__(self):
        return str(self.news_id)



