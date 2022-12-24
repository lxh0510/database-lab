# This is an auto-generated Django model module.
# You'll have to do the following manually to clean this up:
#   * Rearrange models' order
#   * Make sure each model has one field with primary_key=True
#   * Make sure each ForeignKey has `on_delete` set to the desired behavior.
#   * Remove `managed = False` lines if you wish to allow Django to create, modify, and delete the table
# Feel free to rename the models, but don't rename db_table values or field names.
from django.db import models


class Userinfo(models.Model):
    user_id = models.AutoField(primary_key=True, verbose_name='用户编号')
    user_password = models.CharField(max_length=10, verbose_name='用户密码')
    user_phone = models.DecimalField(max_digits=11, decimal_places=0, blank=True, null=True, verbose_name='用户电话')
    user_name = models.CharField(max_length=10, verbose_name='用户名')

    class Meta:
        ordering = ['user_id']
        db_table = 'userinfo'
        verbose_name = "用户信息"
        verbose_name_plural = verbose_name

    def __str__(self):
        return self.user_name
