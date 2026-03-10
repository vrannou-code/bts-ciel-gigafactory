FROM python:3.11-slim

WORKDIR /app
#COPY app.py .

RUN pip install flask ldap3 requests

CMD ["flask", "--app", "app", "run", "--host=0.0.0.0"]
