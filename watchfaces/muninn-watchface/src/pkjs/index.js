const requestMuninnData = () => {
  Pebble.subscribeToSource({
    category: 'muninn',
    item: 'stats',
    limit: 1,
    onData: ({ instances }) => {
      if (!instances.length) return;

      const [data] = instances;
      console.log(JSON.stringify(data));

      const {
        properties: {
          batteryLife: { numericValue: { value: life } },
          daysRemaining: { numericValue: { value: days } },
          lastWeekRate: { numericValue: { value: rate } },
        }
      } = data;

      Pebble.sendAppMessage({
        BATTERY_LIFE: life,
        BATTERY_DAYS_REMAINING: days,
        BATTERY_LAST_WEEK_RATE: rate * 10,
      });
    },
    onError: (err) => console.log(err.code, err.message),
  });
};

Pebble.addEventListener('ready', (e) => {
  console.log('Ready');

  requestMuninnData();
});

Pebble.addEventListener('appmessage', (e) => {
  console.log('appmessage');
  console.log(JSON.stringify(e.dict));
  requestMuninnData();
});
