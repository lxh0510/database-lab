# Generated by Django 2.2 on 2022-12-23 14:31

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('business', '0003_auto_20221223_1419'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='business',
            name='delivery',
        ),
        migrations.RemoveField(
            model_name='business',
            name='minamount',
        ),
    ]
