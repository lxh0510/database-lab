# Generated by Django 2.2 on 2022-12-23 19:56

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('canteenadmin', '0001_initial'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='news',
            name='is_examine',
        ),
        migrations.AddField(
            model_name='canteen',
            name='canteen_graph',
            field=models.ImageField(default=123, upload_to='', verbose_name='食堂图片'),
            preserve_default=False,
        ),
        migrations.AddField(
            model_name='news',
            name='news_title',
            field=models.TextField(default=123, verbose_name='资讯标题'),
            preserve_default=False,
        ),
        migrations.DeleteModel(
            name='AdminNews',
        ),
    ]
