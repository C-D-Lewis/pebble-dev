import { s3Url } from './config';

type S3HotelRoom = {
  name: string;
  remaining: number;
  price: string;
};

type S3HotelItem = {
  name: string;
  rooms: S3HotelRoom[];
};

type S3Data = {
  hotels: S3HotelItem[];
  updatedAt: string;
};

type HotelData = {
  index: number;
  name: string;
  summary: string;
};

type OutputData = {
  hotels: HotelData[];
  updatedAt: string;
};

const getTime = (isoString: string): string => {
  const date = new Date(isoString);
  const hours = date.getHours().toString().padStart(2, '0');
  const minutes = date.getMinutes().toString().padStart(2, '0');
  return `${hours}:${minutes}`;
}

const fetchJsonFile = async (): Promise<OutputData> => {
  const json = await PebbleTS.fetchJSON(s3Url) as S3Data;
  // console.log(JSON.stringify(json, null, 2));

  return {
    hotels: json.hotels
      .filter(hotel => hotel.rooms.some((room: S3HotelRoom) => room.remaining > 0))
        .sort((a, b) => {
          const aCheapest = Math.min(...a.rooms.map(room => parseFloat(room.price)));
          const bCheapest = Math.min(...b.rooms.map(room => parseFloat(room.price)));
          return aCheapest - bCheapest;
        })
        .map((hotel, index) => {
          const summary = hotel.rooms
            .sort((a, b) => parseFloat(a.price) - parseFloat(b.price))
            .map(room => `${room.remaining}x${room.price}`)
            .join(', ');

          return {
            index,
            name: hotel.name,
            summary,
          };
        }),
      updatedAt: getTime(json.updatedAt),
    };
};

const sendNextHotel = async (data: OutputData, index: number) => {
  const { hotels, updatedAt } = data;

  // None
  if (hotels.length === 0) {
    console.log('No hotels to send');
    
    await PebbleTS.sendAppMessage({
      HotelIndex: 0,
      HotelName: 'No rooms found',
      HotelSummary: 'Try again later',
      FlagHotelCount: 1,
      FlagIsComplete: 1,
      FlagUpdatedAt: updatedAt,
    });
    return;
  }

  // Completed
  if (index === hotels.length) {
    console.log('All data sent!');
    return;
  }

  const hotelData = hotels[index];
  if (!hotelData) throw new Error(`No hotelData for ${index}`);

  const dict = {
    HotelIndex: hotelData.index,
    HotelName: hotelData.name,
    HotelSummary: hotelData.summary,
    FlagHotelCount: hotels.length,
    FlagIsComplete: index === hotels.length - 1 ? 1 : 0,
    FlagUpdatedAt: updatedAt,
  };
  await PebbleTS.sendAppMessage(dict);
  console.log(`Sent item ${index}: ${JSON.stringify(dict)}`);

  await sendNextHotel(data, index + 1);
};

Pebble.addEventListener('ready', async (e) => {
  console.log('PebbleKit JS ready');

  try {
    const data = await fetchJsonFile();
    await sendNextHotel(data, 0);
  } catch (e) {
    console.log('Failed to send data');
    console.log(e);
  }
});
