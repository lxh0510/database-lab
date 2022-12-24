# Generated by Django 2.2 on 2022-12-08 22:51

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    initial = True

    dependencies = [
        ('business', '0001_initial'),
    ]

    operations = [
        migrations.CreateModel(
            name='Admininfo',
            fields=[
                ('admin_id', models.AutoField(primary_key=True, serialize=False, verbose_name='管理员编号')),
                ('admin_name', models.CharField(max_length=10, verbose_name='管理员账号名')),
                ('admin_password', models.CharField(max_length=10, verbose_name='管理员密码')),
                ('admin_phone', models.DecimalField(decimal_places=0, max_digits=11, verbose_name='管理员电话')),
            ],
            options={
                'verbose_name': '管理员信息',
                'verbose_name_plural': '管理员信息',
                'db_table': 'admininfo',
                'ordering': ['admin_id'],
            },
        ),
        migrations.CreateModel(
            name='Canteen',
            fields=[
                ('canteen_id', models.AutoField(primary_key=True, serialize=False, verbose_name='食堂编号')),
                ('canteen_name', models.CharField(max_length=5, verbose_name='食堂名称')),
                ('open_time', models.TimeField(verbose_name='开业时间')),
                ('end_time', models.TimeField(verbose_name='结业时间')),
            ],
            options={
                'verbose_name': '食堂信息',
                'verbose_name_plural': '食堂信息',
                'db_table': 'canteen',
                'ordering': ['canteen_id'],
            },
        ),
        migrations.CreateModel(
            name='Category',
            fields=[
                ('category_id', models.AutoField(primary_key=True, serialize=False, verbose_name='种类编号')),
                ('category_name', models.CharField(max_length=10, verbose_name='种类名')),
            ],
            options={
                'verbose_name': '种类信息',
                'verbose_name_plural': '种类信息',
                'db_table': 'category',
                'ordering': ['category_id'],
            },
        ),
        migrations.CreateModel(
            name='News',
            fields=[
                ('news_id', models.AutoField(primary_key=True, serialize=False, verbose_name='咨询编号')),
                ('news_content', models.TextField(verbose_name='资讯内容')),
                ('is_examine', models.IntegerField(choices=[(1, '已审批'), (0, '未审批')], verbose_name='是否审批')),
                ('business', models.ForeignKey(blank=True, null=True, on_delete=django.db.models.deletion.CASCADE, to='business.Business', verbose_name='商家')),
            ],
            options={
                'verbose_name': '美食资讯',
                'verbose_name_plural': '美食资讯',
                'db_table': 'news',
                'ordering': ['news_id'],
            },
        ),
        migrations.CreateModel(
            name='AdminNews',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('examine_time', models.DateTimeField(verbose_name='审批时间')),
                ('examine_result', models.IntegerField(choices=[(1, '已通过'), (0, '未通过')], verbose_name='审批结果')),
                ('admin', models.ForeignKey(on_delete=None, to='canteenadmin.Admininfo', verbose_name='管理员')),
                ('news', models.ForeignKey(on_delete=None, to='canteenadmin.News', verbose_name='资讯')),
            ],
            options={
                'verbose_name': '审批信息',
                'verbose_name_plural': '审批信息',
                'db_table': 'admin_news',
                'ordering': ['-examine_time'],
                'unique_together': {('news', 'admin')},
            },
        ),
    ]
