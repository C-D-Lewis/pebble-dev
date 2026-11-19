Pebble.addEventListener('ready', function (e) {
  console.log('Ready');

  Pebble.subscribeToSource({
    category: 'muninn',
    item: 'stats',
    limit: 1,
    onData: ({ instances }) => {
      console.log(JSON.stringify(instances, null, 2));
    },
    onError: (err) => console.log(err.code, err.message),
  });
});
