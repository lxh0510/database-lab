from django import forms


class BusinessForm(forms.Form):
    business_name = forms.CharField(label="用户名", max_length=128, widget=forms.TextInput(
        attrs={'class': 'form-control', 'placeholder': "Username"}))
    password = forms.CharField(label="密码", max_length=256,
                               widget=forms.PasswordInput(attrs={'class': 'form-control', 'placeholder': "Password"}))


class OrderForm(forms.Form):
    new_name = forms.CharField(label="备注名", widget=forms.TextInput(
        attrs={'class': 'form-control', 'placeholder': "Username"}))
    address = forms.CharField(label="地址", widget=forms.TextInput(
        attrs={'class': 'form-control', 'placeholder': "Address"}))
    arrive_time = forms.CharField(label="送达时间", widget=forms.TextInput(
        attrs={'class': 'form-control', 'placeholder': "Time"}))
    note = forms.CharField(label="备注", widget=forms.TextInput(
        attrs={'class': 'form-control', 'placeholder': "Note"}))


class NewsForm(forms.Form):
    news_title = forms.CharField(label="资讯标题", widget=forms.TextInput(
        attrs={'class': 'form-control', 'placeholder': "NewsTitle"}))
    news_content = forms.CharField(label="资讯内容", widget=forms.Textarea(
        attrs={'class': 'form-control', 'placeholder': "NewsContent"}))
