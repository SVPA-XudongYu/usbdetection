#include "detection.h"

using namespace v8;

#define OBJECT_ITEM_LOCATION_ID     "locationId"
#define OBJECT_ITEM_VENDOR_ID       "vendorId"
#define OBJECT_ITEM_PRODUCT_ID      "productId"
#define OBJECT_ITEM_DEVICE_NAME     "deviceName"
#define OBJECT_ITEM_MANUFACTURER    "manufacturer"
#define OBJECT_ITEM_SERIAL_NUMBER   "serialNumber"
#define OBJECT_ITEM_DEVICE_ADDRESS  "deviceAddress"


v8::Persistent<v8::Value> addedCallback;
bool isAddedRegistered = false;

v8::Persistent<v8::Value> removedCallback;
bool isRemovedRegistered = false;

v8::Handle<v8::Value> RegisterAdded(const v8::Arguments& args) 
{
  v8::HandleScope scope;

  v8::Local<v8::Value> callback;

  if (args.Length() == 0) 
  {
    return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument must be a function"))));
  }

  if (args.Length() == 1) 
  {
    // callback
    if(!args[0]->IsFunction()) 
    {
        return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument must be a function"))));
    }

    callback = args[0];
  }

  addedCallback = v8::Persistent<v8::Value>::New(callback);
  isAddedRegistered = true;

  return scope.Close(v8::Undefined());
}

void NotifyAdded(ListResultItem_t* it) 
{
  if (isAddedRegistered) 
  {
    v8::Handle<v8::Value> argv[1];
    v8::Local<v8::Object> item = v8::Object::New();
    item->Set(v8::String::New(OBJECT_ITEM_LOCATION_ID), v8::Number::New(it->locationId));
    item->Set(v8::String::New(OBJECT_ITEM_VENDOR_ID), v8::Number::New(it->vendorId));
    item->Set(v8::String::New(OBJECT_ITEM_PRODUCT_ID), v8::Number::New(it->productId));
    item->Set(v8::String::New(OBJECT_ITEM_DEVICE_NAME), v8::String::New(it->deviceName.c_str()));
    item->Set(v8::String::New(OBJECT_ITEM_MANUFACTURER), v8::String::New(it->manufacturer.c_str()));
    item->Set(v8::String::New(OBJECT_ITEM_SERIAL_NUMBER), v8::String::New(it->serialNumber.c_str()));
    item->Set(v8::String::New(OBJECT_ITEM_DEVICE_ADDRESS), v8::Number::New(it->deviceAddress));
    argv[0] = item;
    v8::Function::Cast(*addedCallback)->Call(v8::Context::GetCurrent()->Global(), 1, argv);
  }
}

v8::Handle<v8::Value> RegisterRemoved(const v8::Arguments& args) {
  v8::HandleScope scope;

  v8::Local<v8::Value> callback;

  if (args.Length() == 0) 
  {
    return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument must be a function"))));
  }

  if (args.Length() == 1) 
  {
    // callback
    if(!args[0]->IsFunction()) 
    {
        return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument must be a function"))));
    }

    callback = args[0];
  }

  removedCallback = v8::Persistent<v8::Value>::New(callback);
  isRemovedRegistered = true;

  return scope.Close(v8::Undefined());
}

void NotifyRemoved(ListResultItem_t* it) 
{
  if (isRemovedRegistered) 
  {
    v8::Handle<v8::Value> argv[1];
    v8::Local<v8::Object> item = v8::Object::New();
    item->Set(v8::String::New(OBJECT_ITEM_LOCATION_ID), v8::Number::New(it->locationId));
    item->Set(v8::String::New(OBJECT_ITEM_VENDOR_ID), v8::Number::New(it->vendorId));
    item->Set(v8::String::New(OBJECT_ITEM_PRODUCT_ID), v8::Number::New(it->productId));
    item->Set(v8::String::New(OBJECT_ITEM_DEVICE_NAME), v8::String::New(it->deviceName.c_str()));
    item->Set(v8::String::New(OBJECT_ITEM_MANUFACTURER), v8::String::New(it->manufacturer.c_str()));
    item->Set(v8::String::New(OBJECT_ITEM_SERIAL_NUMBER), v8::String::New(it->serialNumber.c_str()));
    item->Set(v8::String::New(OBJECT_ITEM_DEVICE_ADDRESS), v8::Number::New(it->deviceAddress));
    argv[0] = item;
    v8::Function::Cast(*removedCallback)->Call(v8::Context::GetCurrent()->Global(), 1, argv);
  }
}

v8::Handle<v8::Value> Find(const v8::Arguments& args) 
{
  v8::HandleScope scope;

  int vid = 0;
  int pid = 0;
  v8::Local<v8::Value> callback;

  if (args.Length() == 0) 
  {
    return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument must be a function"))));
  }

  if (args.Length() == 3) 
  {
    if (args[0]->IsNumber() && args[1]->IsNumber()) 
    {
        vid = (int) args[0]->NumberValue();
        pid = (int) args[1]->NumberValue();
    }

    // callback
    if(!args[2]->IsFunction()) 
    {
        return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("Third argument must be a function"))));
    }

    callback = args[2];
  }

  if (args.Length() == 2) 
  {
    if (args[0]->IsNumber()) 
    {
        vid = (int) args[0]->NumberValue();
    }

    // callback
    if(!args[1]->IsFunction()) 
    {
        return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("Second argument must be a function"))));
    }

    callback = args[1];
  }

  if (args.Length() == 1) 
  {
    // callback
    if(!args[0]->IsFunction()) 
    {
        return scope.Close(v8::ThrowException(v8::Exception::TypeError(v8::String::New("First argument must be a function"))));
    }

    callback = args[0];
  }

  ListBaton* baton = new ListBaton();
  strcpy(baton->errorString, "");
  baton->callback = v8::Persistent<v8::Value>::New(callback);
  baton->vid = vid;
  baton->pid = pid;

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Find, (uv_after_work_cb)EIO_AfterFind);

  return scope.Close(v8::Undefined());
}

void EIO_AfterFind(uv_work_t* req) 
{
  ListBaton* data = static_cast<ListBaton*>(req->data);

  v8::Handle<v8::Value> argv[2];
  if(data->errorString[0]) 
  {
    argv[0] = v8::Exception::Error(v8::String::New(data->errorString));
    argv[1] = v8::Undefined();
  } 
  else 
  {
    v8::Local<v8::Array> results = v8::Array::New();
    int i = 0;
    for(std::list<ListResultItem_t*>::iterator it = data->results.begin(); it != data->results.end(); it++, i++) {
      v8::Local<v8::Object> item = v8::Object::New();
      item->Set(v8::String::New(OBJECT_ITEM_LOCATION_ID), v8::Number::New((*it)->locationId));
      item->Set(v8::String::New(OBJECT_ITEM_VENDOR_ID), v8::Number::New((*it)->vendorId));
      item->Set(v8::String::New(OBJECT_ITEM_PRODUCT_ID), v8::Number::New((*it)->productId));
      item->Set(v8::String::New(OBJECT_ITEM_DEVICE_NAME), v8::String::New((*it)->deviceName.c_str()));
      item->Set(v8::String::New(OBJECT_ITEM_MANUFACTURER), v8::String::New((*it)->manufacturer.c_str()));
      item->Set(v8::String::New(OBJECT_ITEM_SERIAL_NUMBER), v8::String::New((*it)->serialNumber.c_str()));
      item->Set(v8::String::New(OBJECT_ITEM_DEVICE_ADDRESS), v8::Number::New((*it)->deviceAddress));
      results->Set(i, item);
    }
    argv[0] = v8::Undefined();
    argv[1] = results;
  }
  v8::Function::Cast(*data->callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);

  data->callback.Dispose();
  for(std::list<ListResultItem_t*>::iterator it = data->results.begin(); it != data->results.end(); it++) 
  {
    delete *it;
  }
  delete data;
  delete req;
}

v8::Handle<v8::Value> StartMonitoring(const v8::Arguments& args) 
{
  v8::HandleScope scope;

  Start();

  return scope.Close(v8::Undefined());
}

v8::Handle<v8::Value> StopMonitoring(const v8::Arguments& args) 
{
  v8::HandleScope scope;

  Stop();

  return scope.Close(v8::Undefined());
}

extern "C" {
  void init (v8::Handle<v8::Object> target) 
  {
    v8::HandleScope scope;
    NODE_SET_METHOD(target, "find", Find);
    NODE_SET_METHOD(target, "registerAdded", RegisterAdded);
    NODE_SET_METHOD(target, "registerRemoved", RegisterRemoved);
    NODE_SET_METHOD(target, "startMonitoring", StartMonitoring);
    NODE_SET_METHOD(target, "stopMonitoring", StopMonitoring);
    InitDetection();
  }
}

NODE_MODULE(detection, init);
