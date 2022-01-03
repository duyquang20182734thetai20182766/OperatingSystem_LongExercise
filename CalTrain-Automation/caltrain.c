#include "pintos_thread.h"

struct station {
	// Khai báo các biến chỗ ngồi - seats, người chờ tàu - waiting, khánh đang lên tàu - boarding
	int seats, waiting, boarding;

	// Khai báo khóa bảo vệ biến global
	struct lock lock; 

	// Khai báo biến điều kiện tàu rời đi
	struct condition train_leave; 

	// Khai báo biến điều kiện chỗ ngồi sẵn sàng
	struct condition seat_avalable;
};

// Hàm station_init : Khởi tạo giá trị ban đầu cho struct station
void station_init(struct station *station)
{
	// Khởi tạo các giá trị ban đầu
	station->seats = 0;
	station->waiting = 0;
	station->boarding = 0;

	lock_init(&station->lock);

	// Điều kiện tàu rời đi khi tất cả hành khánh đã lên tàu
	cond_init(&station->train_leave);

	// Điều kiện taù đến khi chỗ ngồi đã sẵn sàng
	cond_init(&station->seat_avalable);
}


// Hàm station_load_train:  Hàm điều kiển các điều kiện để tàu đến ga và mở cửa
void station_load_train(struct station *station, int count)
{
	// Lấy khóa và tiếp tục thực thi hàm
	lock_acquire(&station->lock);

	// Xét giá trị chỗ ngồi
	station->seats = count; 

	// Gửi tín hiệu khách sẵn sàng lên tàu
	cond_broadcast(&station->seat_avalable, &station->lock); 

	//Chỗ ngồi và người chờ > 0 thì thread ngủ
	while (station->seats >0 && station->waiting >0)
	{
		cond_wait(&station->train_leave, &station->lock);
	}

	// Khi chỗ ngồi về 0
	station->seats = 0;
	lock_release(&station->lock);
}

// Hàm station_wait_for_train : Hàm kiểm soát khách đang chờ lên tàu
void station_wait_for_train(struct station *station)
{
	lock_acquire(&station->lock);

	//Xét hành khách chờ tàu tăng lên 1	
	station->waiting ++;

	// Khi mà chỗ ngồi bằng 0 xét tín tiệu cho tàu đến
	while (station->seats == 0)
	{
		cond_wait(&station->seat_avalable, &station->lock);
	}

	//Khi tàu sẵn sàng, mọi người lên tàu boarding++ -> chỗ ngồi giảm xuống seats--
	//Hành khách chờ tàu giảm -> waiting --
	station->boarding ++;
	station->seats --;
	station->waiting --;
	lock_release(&station->lock);
}

// Hàm station_on_board
void station_on_board(struct station *station)
{
	lock_acquire(&station->lock);
	station->boarding--; 

	// Kiểm tra waiting and boading, nếu == 0, cho tàu rời đi
	if((station->seats == 0 || station->waiting == 0) && station->boarding == 0)
	{
		cond_signal(&station->train_leave, &station->lock);
	}
	
	lock_release(&station->lock);
}
