from tornado import websocket, web, ioloop
import json

cl = []

class IndexHandler(web.RequestHandler):
    def get(self):
        self.render("index.html")

class SocketHandler(websocket.WebSocketHandler):
    def check_origin(self, origin):
        return True

    def open(self):
        if self not in cl:
            cl.append(self)

    def on_message(self, message):
        pass

    def on_close(self):
        if self in cl:
            cl.remove(self)

class ApiHandler(web.RequestHandler):

    @web.asynchronous
    def get(self, *args):
        self.finish()
        id = self.get_argument("id")
        value = self.get_argument("value")
        data = {"id": id, "value" : value}
        data = json.dumps(data)
        for c in cl:
            c.write_message(data)

    @web.asynchronous
    def post(self):
        pass

class StartServer():
    def __init__(self,on_message):
        SocketHandler.on_message = on_message


    def serve(self):
        try:
            app = web.Application([
                (r'/', IndexHandler),
                (r'/ws', SocketHandler),
                (r'/api', ApiHandler),])
            app.listen(8080)
            ioloop.IOLoop.instance().start()
        except ALIVE:
            print "Shutting down.."
