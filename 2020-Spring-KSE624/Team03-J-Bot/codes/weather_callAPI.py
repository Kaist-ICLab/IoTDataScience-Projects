#!/usr/bin/env python
# coding: utf-8

'''
Final Project for KSE624 Mobile and Pervasive Computing for Knowledge Services Spring 2020 at KAIST

Last Updated Date: July 01 2020
Authors: 
    Rafikatiwi Nur Pujiarti
    Willmer R. Quinones

-----------------------------

weather_callAPI.py

(1) collect_data: 
    - Collect the weather information and air quality from the API's
(2) filter_weather_data: 
    - Extract temperature and weather condition (current and forecasted) from the weather data 
      extracted from the OpenWeather API
(3) filter_air_data: 
    - Extract air quality information from the AirVisual API
(4) convert_time_data: 
    - Convert time data information to string, assigning AM and PM
(5) convert_weather_condition_data:
    - Standarize the weather condition for J-Bot to communicate user-friendly
(6) get_outside_condition:
    - Get the data from the OpenWeather API and process them accordingly

'''

## Necessary Packages
import requests
import json
import geocoder
from datetime import datetime
import copy

def collect_data(api_key_weather, api_key_air):

    '''

    J-Bot uses OpenWeather API to get the weather forecast:
        https://openweathermap.org/api

    And AirVisual API to get the air quality:
        https://www.iqair.com/air-pollution-data-api

    Please sign-up to those pages to get your own keys

    -----------------------------

    This function returns the weather and air quality information from the APIs

    Args:
        - api_key_weather: (string) your key for the OpenWeather API
        - api_key_air: (string) your key for the AirVisual API
    Returns:
        - collected_data: (dict) the information related to the weather and the air quality

    '''
    
    # Getting your current geolocation in latitude and longitude
    g = geocoder.ip('me')
    lat = str(g.latlng[0])
    lon = str(g.latlng[1])

    collected_data = {}

    # The Weather and AirQuality API urls
    weather_url = "https://api.openweathermap.org/data/2.5/onecall?lat=%s&lon=%s&appid=%s&units=metric" % (lat, lon, api_key_weather)
    air_url = f"http://api.airvisual.com/v2/nearest_city?key={api_key_air}"
    
    weather_response = requests.get(weather_url)
    weather_data = json.loads(weather_response.text)
    weather_data_filtered = filter_weather_data(weather_data)
    
    air_response = requests.get(air_url)
    iqair_data = json.loads(air_response.text)
    iqair_data_filtered = filter_air_data(iqair_data)
    
    # Getting all the information from the OpenWeather API and AirVisual API
    collected_data = copy.deepcopy(weather_data_filtered)
    collected_data['air_condition'] = iqair_data_filtered
    
    return collected_data

def filter_weather_data(weather_d):

    '''

    Extract the meaningful information from the weather data extracted from the OpenWeather API
        1. Temperature (both measured and 'feels like' temperatures)
        2. Weather condition (raining, sunny, snowing)
    for the current time and for the next 24 hours (hourly)

    Args:
        - weather_d: (dict) The weather information gotten from the API
    Returns:
        - current_weather_filtered: (dict) the information related to the weather and the air quality
            - time: current time
            - temperature: value of the measured temperature
            - feels_like: value of the 'feels like' temperature
            - weather: (str) weather category (raining, cloudy, snowing...)
            - forecast: forcasted weather condition

    '''

    # Extract the weather information for the next 48 hours
    hourly_forecast = weather_d['hourly'][13]['dt'] 

    # Extract the current weather info
    current_hour = datetime.fromtimestamp(weather_d['hourly'][0]['dt']).hour
    current_weather = weather_d['current']

    # Hourly forecast until the end of day, consist of (24 - x) hours
    hourly_forecast_today = weather_d['hourly'][1:(24 - current_hour)] 
    
    # Extracting only the time, temperature (measured and 'feels like'), and weather condition
    # for both current situation and forecasted
    current_weather_filtered = {}
    current_weather_filtered['time'] = datetime.fromtimestamp(current_weather['dt']).hour
    current_weather_filtered['temperature'] = current_weather['temp']
    current_weather_filtered['feels_like'] = current_weather['feels_like']
    current_weather_filtered['weather'] = {'main': current_weather['weather'][0]['main'], 'detail': current_weather['weather'][0]['description']}    

    hourly_forecast_filtered = [{'time': datetime.fromtimestamp(h_forecast['dt']).hour, 'temperature': h_forecast['temp'], 'feels_like': h_forecast['feels_like'],
                                 'weather': {'main': h_forecast['weather'][0]['main'], 'detail': h_forecast['weather'][0]['description']}} for h_forecast in hourly_forecast_today]
    
    current_weather_filtered['forecast'] = hourly_forecast_filtered

    return current_weather_filtered

def filter_air_data(air_d):

    '''

    Extract the meaningful information from the air quality data extracted from the AirVisual API

    Args:
        - air_d: (dict) The air quality information gotten from the API
    Returns:
        - aqair_today: (dict) air quality situation for today
            - aqius: (int) the air quality index
            - air_pollution_level: (str) air quality category

    '''

    aqius_today = air_d['data']['current']['pollution']['aqius']
    air_pollution_level = ""
    if (aqius_today >= 0) and (aqius_today <= 50):
        air_pollution_level = 'Good'
    elif (aqius_today >= 51) and (aqius_today <= 100):
        air_pollution_level = 'Moderate'
    elif (aqius_today >= 101) and (aqius_today <= 150):
        air_pollution_level = 'Unhealthy for Sensitive Groups'
    elif (aqius_today >= 151) and (aqius_today <= 200):
        air_pollution_level = 'Unhealthy'
    elif (aqius_today >= 201) and (aqius_today <= 300):
        air_pollution_level = 'Very Unhealthy'
    elif (aqius_today >= 300):
        air_pollution_level = 'Hazardous'

    aqair_today = {'aqius': aqius_today, 'level': air_pollution_level}
    return aqair_today

def convert_time_data(d):

    '''

    Convert time data information to string, assigning AM and PM

    Args:
        - d: (dict) The weather information gotten from the API
    Returns:
        - d: (dict) The modified weather information with the time data converted to string

    '''

    # Convert current time data
    if d['time'] > 12:
        d['time'] = d['time'] - 12
        d['time'] = str(d['time']) + " PM"
    else:
        d['time'] = str(d['time']) + " AM"

    # Convert forecast times data
    for f in d['forecast']:
        if f['time'] > 12:
            f['time'] = f['time'] - 12
            f['time'] = str(f['time']) + " PM"
        else:
            f['time'] = str(f['time']) + " AM"
            
    return d

def convert_weather_condition_data(d):

    '''

    Standarizing the weather condition for J-Bot to communicate user-friendly
        e.g. 'Drizzle' to 'raining' or 'showers', 'Snow' to 'snowing'...

    Args:
        - d: (dict) The weather information gotten from the API
    Returns:
        - d: (dict) The modified weather information with the standarized weather conditions

    '''

    # Handing current information
    if d['weather']['main'] == 'Drizzle':
        if d['weather']['detail'] in 'heavy':
            d['weather']['detail'] = 'raining'
        else:
            d['weather']['detail'] = 'showers'
    elif d['weather']['main'] == 'Rain':
        if d['weather']['detail'] in 'heavy' or d['weather']['detail'] in 'extreme':
            d['weather']['detail'] = 'raining'
        elif d['weather']['detail'] in 'freezing':
            d['weather']['detail'] = 'snowing'
        else:
            d['weather']['detail'] = 'raining'
    elif d['weather']['main'] == 'Snow':
        d['weather']['detail'] = 'snowing'
    elif d['weather']['main'] == 'Clear':
        d['weather']['detail'] = 'clear'
    elif d['weather']['main'] == 'Clouds':
        d['weather']['detail'] = 'cloudy'
    elif d['weather']['main'] in ['Mist', 'Smoke', 'Haze', 'Dust', 'Fog', 'Sand', 'Dust', 'Ash', 'Squall', 'Tornado']:
        d['weather']['detail'] = d['weather']['main'].lower()
        
    # Handling forecasted information
    for f in d['forecast']:
        if f['weather']['main'] == 'Drizzle':
            if f['weather']['detail'] in 'heavy':
                f['weather']['detail'] = 'raining'
            else:
                f['weather']['detail'] = 'showers'
        elif f['weather']['main'] == 'Rain':
            if f['weather']['detail'] in 'heavy' or f['weather']['detail'] in 'extreme':
                f['weather']['detail'] = 'raining'
            elif f['weather']['detail'] in 'freezing':
                f['weather']['detail'] = 'snowing'
            else:
                f['weather']['detail'] = 'raining'
        elif f['weather']['main'] == 'Snow':
            f['weather']['detail'] = 'snowing'
        elif f['weather']['main'] == 'Clear':
            f['weather']['detail'] = 'clear'
        elif f['weather']['main'] == 'Clouds':
            f['weather']['detail'] = 'cloudy'
        elif f['weather']['main'] in ['Mist', 'Smoke', 'Haze', 'Dust', 'Fog', 'Sand', 'Dust', 'Ash', 'Squall', 'Tornado']:
            f['weather']['detail'] = f['weather']['main'].lower()
        
    return d

def get_outside_condition():

    '''

    J-Bot uses OpenWeather API to get the weather forecast:
        https://openweathermap.org/api

    And AirVisual API to get the air quality:
        https://www.iqair.com/air-pollution-data-api

    Please sign-up to those pages to get your own keys

    -----------------------------

    Getting the data from the OpenWeather API and processing them accordingly

    Returns:
        - d: (dict) Processed weather information

    '''

    api_key_ow = '' # <Your subscription key> 
    api_key_iq = '' # <Your subscription key>

    d = collect_data(api_key_ow, api_key_iq)
    d = convert_time_data(d)
    d = convert_weather_condition_data(d)

    return d

